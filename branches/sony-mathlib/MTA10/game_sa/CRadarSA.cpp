/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRadarSA.cpp
*  PURPOSE:     Game radar
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Alberto Alonso <rydencillo@gmail.com>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

struct CRect {
    float fX1, fY1, fX2, fY2;
};

CMarkerSA* Markers[MAX_MARKERS];

CRadarSA::CRadarSA( void )
{
	for( int i = 0; i < MAX_MARKERS; i++ )
		Markers[i] = new CMarkerSA((CMarkerSAInterface *)(ARRAY_CMarker + i * sizeof(CMarkerSAInterface)));
}

CRadarSA::~CRadarSA( void )
{
	for( int i = 0; i < MAX_MARKERS; i++ )
        if ( Markers[i] ) delete Markers[i];
}

CMarker* CRadarSA::CreateMarker( const CVector& vecPosition )
{
	CMarkerSA* marker;
	marker = (CMarkerSA *)this->GetFreeMarker();
    if ( marker )
    {
        marker->Init ();
	    marker->SetPosition( vecPosition );
    }

/*	DWORD dwFunc = FUNC_SetCoordBlip;
	float fX = vecPosition->getX(); float fY = vecPosition->getY();	float fZ = vecPosition->getZ();
	_asm {
		push 0
		push 2
		push 0
		push fZ
		push fY
		push fX
		push 4
		call dwFunc
		add esp, 0x1C
	}*/
	//marker->SetColor((eMarkerColor)MARKER_COLOR_PALE_GREEN);
	//marker->SetScale(MARKER_SCALE_NORMAL);
	//marker->SetSprite((eMarkerSprite)MARKER_SPRITE_BORDER);
	//marker->SetDisplay((eMarkerDisplay)MARKER_DISPLAY_MARKERONLY ); 

	return marker;
}

CMarker* CRadarSA::CreateMarker( CVehicle * vehicle )
{
	CMarkerSA* marker;
	marker = (CMarkerSA *)this->GetFreeMarker();

	marker->SetEntity(vehicle);
	marker->SetColor((eMarkerColor)MARKER_COLOR_YELLOW);
	marker->SetScale(MARKER_SCALE_NORMAL);
	marker->SetSprite((eMarkerSprite)MARKER_SPRITE_NONE);
	marker->SetDisplay((eMarkerDisplay)MARKER_DISPLAY_MARKERONLY); 

	return marker;
}

CMarker* CRadarSA::CreateMarker( CObject * object )
{
	CMarkerSA* marker;
	marker = (CMarkerSA *)this->GetFreeMarker();

	marker->SetEntity(object);
	marker->SetColor((eMarkerColor)MARKER_COLOR_YELLOW);
	marker->SetScale(MARKER_SCALE_NORMAL);
	marker->SetSprite((eMarkerSprite)MARKER_SPRITE_NONE);
	marker->SetDisplay((eMarkerDisplay)MARKER_DISPLAY_MARKERONLY); 

	return marker;
}

CMarker* CRadarSA::CreateMarker( CPed * ped )
{
	CMarkerSA* marker;
	marker = (CMarkerSA *)this->GetFreeMarker();

	marker->SetEntity(ped);
	marker->SetColor((eMarkerColor)MARKER_COLOR_YELLOW);
	marker->SetScale(MARKER_SCALE_NORMAL);
	marker->SetSprite((eMarkerSprite)MARKER_SPRITE_NONE);
	marker->SetDisplay((eMarkerDisplay)MARKER_DISPLAY_MARKERONLY);

	return marker;
}

CMarker* CRadarSA::GetFreeMarker( void )
{
	int Index;
	Index = 0;
	while ((Index < MAX_MARKERS) && (Markers[Index]->GetInterface()->bTrackingBlip))
	{
		Index++;
	}
	if ( Index >= MAX_MARKERS )
		return NULL;
	else
		return Markers[Index];
}

CMarker	* CRadarSA::GetMarker( DWORD dwMarkerID )
{
	return (CMarker *)Markers[dwMarkerID];
}

void CRadarSA::ClearMarkerForEntity( CVehicle* vehicle )
{
	CMarkerSA* marker;
	for( int i = 0; i < MAX_MARKERS; i++ )
	{
		marker = (CMarkerSA *)(ARRAY_CMarker + i * sizeof(CMarkerSA));
		if(marker->GetEntity() == (CEntity *)vehicle)
		{
			marker->Remove();
		}
	}
}

void CRadarSA::ClearMarkerForEntity( CObject* object )
{
	CMarkerSA* marker;
	for( int i = 0; i < MAX_MARKERS; i++ )
	{
		marker = (CMarkerSA *)(ARRAY_CMarker + i * sizeof(CMarkerSA));
		if(marker->GetEntity() == (CEntity *)object)
		{
			marker->Remove();
		}
	}
}

void CRadarSA::ClearMarkerForEntity( CPed* ped )
{
	CMarkerSA* marker;
	for( int i = 0; i < MAX_MARKERS; i++ )
	{
		marker = (CMarkerSA *)(ARRAY_CMarker + i * sizeof(CMarkerSA));
		if(marker->GetEntity() == (CEntity *)ped)
		{
			marker->Remove();
		}
	}
}

void CRadarSA::DrawAreaOnRadar( float fX1, float fY1, float fX2, float fY2, DWORD abgr )
{
    CRect myRect;
    myRect.fX1 = fX1;
    myRect.fY1 = fY1;
    myRect.fX2 = fX2;
    myRect.fY2 = fY2;
    DWORD dwFunc = FUNC_DrawAreaOnRadar;
    _asm
    {
        push    eax

        push    0
        lea     eax, abgr
        push    eax
        lea     eax, myRect
        push    eax
        call    dwFunc
        add     esp, 12

        pop     eax
    }
}