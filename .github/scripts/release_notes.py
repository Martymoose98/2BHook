"""Release notes step of release.yml.

Reads CHANGELOG.md (a copy of .github/release-notes/<tag>.md, or the changelog generated from the
commits) and:
  - takes out the optional nexus-summary block and checks it. It becomes the `summary` output,
    the Nexus file description. Without a block, the summary is made from the notes' opening
    paragraph, trimmed to fit.
  - writes the `nexus` output: the plain-text changelog for the Nexus mod page.
  - rewrites CHANGELOG.md without the block, for the GitHub release.

The block can go anywhere in the notes file. Its lines are joined with spaces:

    ```nexus-summary
    November 2026 Update. Supports the new Steam build and v1.02 unpacked.
    ```

Run from the repository root with TAG and GITHUB_OUTPUT set (GITHUB_REPOSITORY and
GITHUB_SERVER_URL for the link to the full notes).
"""
import os
import re
import sys
import uuid

MAX_SUMMARY = 255   # Nexus's limit for a file description
NOTES = 'CHANGELOG.md'

TAG = os.environ['TAG']
SOURCE = f'.github/release-notes/{TAG}.md'
BLOCK = re.compile(r'^```nexus-summary[ \t]*\n(.*?)^```[ \t]*(?:\n|\Z)', re.M | re.S)
OPENING_FENCE = re.compile(r'^```nexus-summary', re.M)


def length(s):
    """Length as the Nexus API counts it: UTF-16 code units, like JavaScript's String.length."""
    return len(s.encode('utf-16-le')) // 2


def fail(message):
    print(f'::error file={SOURCE}::{message}')
    sys.exit(1)


def plain(s):
    """Markdown to plain text: links keep their text, bold and code marks go."""
    s = re.sub(r'\[([^\]]*)\]\([^)]*\)', r'\1', s)
    return s.replace('**', '').replace('`', '')


def trim(s, limit=MAX_SUMMARY):
    """Cut s to fit the limit at a word boundary, ending in '...'."""
    if length(s) <= limit:
        return s
    s = s[:limit - 3]
    while length(s) > limit - 3:
        s = s[:-1]
    if ' ' in s:
        s = s.rsplit(' ', 1)[0]
    return s.rstrip(' ,;:.-') + '...'


def flatten(paragraph):
    """One line from a paragraph: bullet items joined with '; ', other lines with spaces."""
    lines = [line.strip() for line in paragraph.splitlines() if line.strip()]
    bullets = all(re.match(r'[-*] ', line) for line in lines)
    lines = [re.sub(r'^[-*] ', '', line) for line in lines]
    return plain(('; ' if bullets else ' ').join(lines))


def summary_from_notes(text):
    """The notes' opening paragraph (headings skipped), or failing that their first heading."""
    paragraphs = [p for p in re.split(r'\n[ \t]*\n', text) if p.strip()]
    for p in paragraphs:
        if not p.lstrip().startswith(('#', '```')):
            return trim(' '.join(flatten(p).split()))
    for p in paragraphs:
        if p.lstrip().startswith('#'):
            return trim(plain(p.strip().splitlines()[0].lstrip('#').strip()))
    return f'2BHook {TAG}'


def nexus_changelog(text):
    """The changes as plain-text lines: every bullet except those under Known issues and
    Install, then a link to the full notes."""
    lines, skip = [], False
    for line in text.splitlines():
        if re.match(r'#+ ', line):
            skip = bool(re.match(r'#+ (Known issues|Install)\b', line))
        elif not skip and line.startswith('- '):
            lines.append(plain(line[2:].strip()))
    server = os.environ.get('GITHUB_SERVER_URL', 'https://github.com')
    repo = os.environ.get('GITHUB_REPOSITORY', 'Martymoose98/2BHook')
    lines.append(f'Full release notes: {server}/{repo}/releases/tag/{TAG}')
    return '\n'.join(lines)


def write_output(name, value):
    delimiter = f'EOF_{uuid.uuid4().hex}'
    with open(os.environ['GITHUB_OUTPUT'], 'a', encoding='utf-8', newline='\n') as out:
        out.write(f'{name}<<{delimiter}\n{value}\n{delimiter}\n')


def main():
    # The runner's pipe would otherwise be cp1252 on Windows, which can't print every character
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')

    with open(NOTES, encoding='utf-8') as f:     # universal newlines: CRLF reads as \n
        text = f.read()

    blocks = BLOCK.findall(text)
    if len(blocks) > 1:
        fail(f'There are {len(blocks)} nexus-summary blocks. Keep one.')
    if not blocks and OPENING_FENCE.search(text):
        fail('The nexus-summary block is not closed. End it with a line of three backticks.')

    if blocks:
        summary = ' '.join(blocks[0].split())
        if not summary:
            fail('The nexus-summary block is empty. Write the summary or remove the block.')
        n = length(summary)
        if n > MAX_SUMMARY:
            fail(f'The nexus-summary is {n} characters, and Nexus allows {MAX_SUMMARY}. '
                 f'Shorten it by {n - MAX_SUMMARY}.')
        text = re.sub(r'\n{3,}', '\n\n', BLOCK.sub('', text)).strip() + '\n'
        origin = 'the nexus-summary block'
    else:
        summary = summary_from_notes(text)
        origin = "the notes' opening paragraph (no nexus-summary block)"

    with open(NOTES, 'w', encoding='utf-8', newline='\n') as f:
        f.write(text)

    write_output('summary', summary)
    write_output('nexus', nexus_changelog(text))

    print(f'::notice::Nexus file description, {length(summary)} characters, from {origin}: {summary}')
    print(text)


if __name__ == '__main__':
    main()
