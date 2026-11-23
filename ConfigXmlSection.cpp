#include "ConfigXmlSection.h"

#include <Shlwapi.h>
#include <wrl.h>

#include <string>

#pragma comment(lib, "xmllite.lib")
#pragma comment(lib, "shlwapi.lib")

using Microsoft::WRL::ComPtr;

namespace
{
	// Writes the element the reader is on, with its attributes. Empty elements (<a/>) are
	// closed straight away; the rest are closed when their end element is copied.
	HRESULT CopyElementStart(IXmlReader* pReader, IXmlWriter* pWriter, bool bEmpty)
	{
		HRESULT hr;
		LPCWSTR szPrefix = NULL, szName = NULL, szNamespace = NULL;

		if (FAILED(hr = pReader->GetPrefix(&szPrefix, NULL)) ||
			FAILED(hr = pReader->GetLocalName(&szName, NULL)) ||
			FAILED(hr = pReader->GetNamespaceUri(&szNamespace, NULL)))
			return hr;

		if (FAILED(hr = pWriter->WriteStartElement((szPrefix && *szPrefix) ? szPrefix : NULL, szName,
			(szNamespace && *szNamespace) ? szNamespace : NULL)))
			return hr;

		for (hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
		{
			LPCWSTR szAttrPrefix = NULL, szAttrName = NULL, szAttrNamespace = NULL, szValue = NULL;
			HRESULT hrAttr;

			if (FAILED(hrAttr = pReader->GetPrefix(&szAttrPrefix, NULL)) ||
				FAILED(hrAttr = pReader->GetLocalName(&szAttrName, NULL)) ||
				FAILED(hrAttr = pReader->GetNamespaceUri(&szAttrNamespace, NULL)) ||
				FAILED(hrAttr = pReader->GetValue(&szValue, NULL)) ||
				FAILED(hrAttr = pWriter->WriteAttributeString((szAttrPrefix && *szAttrPrefix) ? szAttrPrefix : NULL,
					szAttrName, (szAttrNamespace && *szAttrNamespace) ? szAttrNamespace : NULL, szValue)))
				return hrAttr;
		}

		if (FAILED(hr))
			return hr;

		if (FAILED(hr = pReader->MoveToElement()))
			return hr;

		return bEmpty ? pWriter->WriteEndElement() : S_OK;
	}

	// The copy itself: every node but the old section, with the new one in its place (or last)
	HRESULT CopyWithSection(IXmlReader* pReader, IXmlWriter* pWriter, LPCWSTR szSection,
		const std::function<HRESULT(IXmlWriter*)>& WriteSection)
	{
		// XmlLite gives an end element the depth of its content, one more than its start
		// element's: <Root> is at 0 and </Root> at 1, a section's end at 2.
		constexpr UINT ROOT_END_DEPTH = 1;
		constexpr UINT SECTION_END_DEPTH = 2;

		HRESULT hr;
		XmlNodeType Type;
		bool bFound = false;
		bool bSkipping = false;

		if (FAILED(hr = pWriter->WriteStartDocument(XmlStandalone_Omit)))
			return hr;

		// Read returns S_FALSE at the end of the document
		while ((hr = pReader->Read(&Type)) == S_OK)
		{
			UINT uDepth = 0;

			if (FAILED(hr = pReader->GetDepth(&uDepth)))
				return hr;

			if (bSkipping)
			{
				if (Type == XmlNodeType_EndElement && uDepth == SECTION_END_DEPTH)
					bSkipping = false;

				continue;
			}

			switch (Type)
			{
			case XmlNodeType_Element:
			{
				// Has to be asked before moving to the attributes
				const bool bEmpty = pReader->IsEmptyElement() != FALSE;
				LPCWSTR szName = NULL;

				if (FAILED(hr = pReader->GetLocalName(&szName, NULL)))
					return hr;

				if (uDepth == 1 && szName && !wcscmp(szName, szSection))
				{
					if (bFound)
					{
						// A second copy of the section is dropped
						bSkipping = !bEmpty;
						break;
					}

					bFound = true;
					bSkipping = !bEmpty;

					if (FAILED(hr = WriteSection(pWriter)))
						return hr;

					break;
				}

				if (uDepth == 0 && bEmpty)
				{
					// <Root/>: open it, add the section, close it
					if (FAILED(hr = CopyElementStart(pReader, pWriter, false)) ||
						FAILED(hr = WriteSection(pWriter)) ||
						FAILED(hr = pWriter->WriteFullEndElement()))
						return hr;

					bFound = true;
					break;
				}

				if (FAILED(hr = CopyElementStart(pReader, pWriter, bEmpty)))
					return hr;

				break;
			}

			case XmlNodeType_EndElement:
				// The section goes last when the file didn't have one
				if (uDepth == ROOT_END_DEPTH && !bFound)
				{
					if (FAILED(hr = WriteSection(pWriter)))
						return hr;

					bFound = true;
				}

				if (FAILED(hr = pWriter->WriteFullEndElement()))
					return hr;

				break;

			case XmlNodeType_Text:
			case XmlNodeType_CDATA:
			case XmlNodeType_Comment:
			{
				LPCWSTR szValue = NULL;

				if (FAILED(hr = pReader->GetValue(&szValue, NULL)))
					return hr;

				if (Type == XmlNodeType_Text)
					hr = pWriter->WriteString(szValue);
				else if (Type == XmlNodeType_CDATA)
					hr = pWriter->WriteCData(szValue);
				else
					hr = pWriter->WriteComment(szValue);

				if (FAILED(hr))
					return hr;

				break;
			}

			case XmlNodeType_ProcessingInstruction:
			{
				LPCWSTR szTarget = NULL, szValue = NULL;

				if (FAILED(hr = pReader->GetLocalName(&szTarget, NULL)) ||
					FAILED(hr = pReader->GetValue(&szValue, NULL)) ||
					FAILED(hr = pWriter->WriteProcessingInstruction(szTarget, szValue)))
					return hr;

				break;
			}

			default:
				// The XML declaration (WriteStartDocument wrote a new one), whitespace (the
				// writer indents) and document types (prohibited by the reader anyway)
				break;
			}
		}

		if (FAILED(hr))
			return hr;

		// A document with no root element at all
		if (!bFound)
			return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

		if (FAILED(hr = pWriter->WriteEndDocument()))
			return hr;

		return pWriter->Flush();
	}
}

HRESULT RewriteXmlSection(LPCWSTR szPath, LPCWSTR szSection, const std::function<HRESULT(IXmlWriter*)>& WriteSection)
{
	if (!szPath || !szSection || !WriteSection)
		return E_INVALIDARG;

	if (GetFileAttributesW(szPath) == INVALID_FILE_ATTRIBUTES)
	{
		const DWORD dwError = GetLastError();

		// Only a file that isn't there reads as missing: SaveSection then creates the file,
		// truncating whatever a lookup that failed for another reason couldn't see
		if (dwError == ERROR_FILE_NOT_FOUND || dwError == ERROR_PATH_NOT_FOUND)
			return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

		return HRESULT_FROM_WIN32(dwError);
	}

	const std::wstring TempPath = std::wstring(szPath) + L".tmp";
	HRESULT hr;

	{
		ComPtr<IStream> pInput, pOutput;
		ComPtr<IXmlReader> pReader;
		ComPtr<IXmlWriter> pWriter;

		if (FAILED(hr = SHCreateStreamOnFileEx(szPath, STGM_READ | STGM_SHARE_DENY_WRITE, FILE_ATTRIBUTE_NORMAL,
			FALSE, NULL, &pInput)))
			return hr;

		if (FAILED(hr = CreateXmlReader(IID_IXmlReader, (void**)&pReader, NULL)) ||
			FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)) ||
			FAILED(hr = pReader->SetInput(pInput.Get())))
			return hr;

		if (FAILED(hr = SHCreateStreamOnFileEx(TempPath.c_str(), STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE,
			FILE_ATTRIBUTE_NORMAL, TRUE, NULL, &pOutput)))
			return hr;

		if (SUCCEEDED(hr = CreateXmlWriter(IID_IXmlWriter, (void**)&pWriter, NULL)) &&
			SUCCEEDED(hr = pWriter->SetOutput(pOutput.Get())) &&
			SUCCEEDED(hr = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE)))
		{
			hr = CopyWithSection(pReader.Get(), pWriter.Get(), szSection, WriteSection);
		}

		// Both files are closed when the streams go out of scope here, before the move
	}

	if (SUCCEEDED(hr) && !MoveFileExW(TempPath.c_str(), szPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
		hr = HRESULT_FROM_WIN32(GetLastError());

	if (FAILED(hr))
		DeleteFileW(TempPath.c_str());

	return hr;
}
