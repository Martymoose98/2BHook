#pragma once

static bool* GetModelGravity()
{
	if (!g_pCamera)
		return NULL;

	Pl0000* pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);

	if (!pCameraEnt)
		return NULL;

	return (bool*)&pCameraEnt->m_obb.m_bGravity;
}

static float* GetOBBY()
{
	if (!g_pCamera)
		return NULL;

	Pl0000* pCameraEnt = GetEntityFromHandle(&g_pCamera->m_hEntity);

	if (!pCameraEnt)
		return NULL;

	return &pCameraEnt->m_obb.m_vecs[0].y;
}