/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPhysicalSA.cpp
*  PURPOSE:     Physical object entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"

const CVector CPhysicalSA::GetMoveSpeed( void )
{
	CVectorGTA *vec;
	DWORD dwFunc = FUNC_GetMoveSpeed;
	DWORD dwThis = (DWORD)((CPhysicalSAInterface *)this->GetInterface());
	_asm
	{
		mov		ecx, dwThis
		call	dwFunc
		mov		vec, eax
	}
	return CVectorGTA::unwrap( *vec );
}

const CVector CPhysicalSA::GetTurnSpeed( void )
{
	CVectorGTA *vec;
	DWORD dwFunc = FUNC_GetTurnSpeed;
	DWORD dwThis = (DWORD)((CPhysicalSAInterface *)this->GetInterface());
	_asm
	{
		mov		ecx, dwThis
		call	dwFunc
		mov		vec, eax
	}
	return CVectorGTA::unwrap( *vec );
}

void CPhysicalSA::SetMoveSpeed( const CVector& vecMoveSpeed )
{
	CVectorGTA *vec;
	DWORD dwFunc = FUNC_GetMoveSpeed;
	DWORD dwThis = (DWORD)((CPhysicalSAInterface *)this->GetInterface());
	DWORD dwReturn = 0;
	_asm
	{
		mov		ecx, dwThis
		call	dwFunc
		mov		dwReturn, eax
	}
	*vec = vecMoveSpeed;
}

void CPhysicalSA::SetTurnSpeed( const CVector& vecTurnSpeed )
{
	CVectorGTA *vec;
	DWORD dwFunc = FUNC_GetTurnSpeed;
	DWORD dwThis = (DWORD)((CPhysicalSAInterface *)this->GetInterface());
	_asm
	{
		mov		ecx, dwThis
		call	dwFunc
		mov		vec, eax
	}
	*vec = vecTurnSpeed;
}

float CPhysicalSA::GetMass ( void )
{
    return ((CPhysicalSAInterface *)this->GetInterface())->fMass;
}


void CPhysicalSA::SetMass ( float fMass )
{
    ((CPhysicalSAInterface *)this->GetInterface())->fMass = fMass;
}


float CPhysicalSA::GetTurnMass ( void )
{
    return ((CPhysicalSAInterface *)this->GetInterface())->fTurnMass;
}


void CPhysicalSA::SetTurnMass ( float fTurnMass )
{
    ((CPhysicalSAInterface *)this->GetInterface())->fTurnMass = fTurnMass;
}


float CPhysicalSA::GetElasticity ( void )
{
    return ((CPhysicalSAInterface *)this->GetInterface())->fElasticity;
}


void CPhysicalSA::SetElasticity ( float fElasticity )
{
    ((CPhysicalSAInterface *)this->GetInterface())->fElasticity = fElasticity;
}


float CPhysicalSA::GetBuoyancyConstant ( void )
{
    return ((CPhysicalSAInterface *)this->GetInterface())->fBuoyancyConstant;
}


void CPhysicalSA::SetBuoyancyConstant ( float fBuoyancyConstant )
{
    ((CPhysicalSAInterface *)this->GetInterface())->fBuoyancyConstant = fBuoyancyConstant;
}


void CPhysicalSA::ProcessCollision()
{
	DEBUG_TRACE("void CPhysicalSA::ProcessCollision()");
	DWORD dwFunc = FUNC_ProcessCollision;
	DWORD dwThis = (DWORD)this->GetInterface();

	_asm
	{
		mov		ecx, dwThis
		call	dwFunc
	}
}

float CPhysicalSA::GetDamageImpulseMagnitude ( void )
{
    return ((CPhysicalSAInterface *)this->GetInterface())->fDamageImpulseMagnitude;
}


void CPhysicalSA::SetDamageImpulseMagnitude ( float fMagnitude )
{
    ((CPhysicalSAInterface *)this->GetInterface())->fDamageImpulseMagnitude = fMagnitude;
}


CEntity* CPhysicalSA::GetDamageEntity ( void )
{
    // TODO: CPhysicalSA::GetDamageEntity
    return NULL;
}


void CPhysicalSA::SetDamageEntity ( CEntity* pEntity )
{
    CEntitySA* pEntitySA = dynamic_cast < CEntitySA* > ( pEntity );
	if ( pEntitySA )
		((CPhysicalSAInterface *)this->GetInterface())->damageEntity = pEntitySA->GetInterface ();
}


void CPhysicalSA::ResetLastDamage ( void )
{
    ((CPhysicalSAInterface *)this->GetInterface())->fDamageImpulseMagnitude = 0.0f;
    ((CPhysicalSAInterface *)this->GetInterface())->damageEntity = NULL;
}

CEntity * CPhysicalSA::GetAttachedEntity ( void )
{
    CEntitySAInterface * pInterface = ((CPhysicalSAInterface *)this->GetInterface())->pAttachedEntity;
    CPoolsSA * pPools = ((CPoolsSA *)pGame->GetPools());
	CEntity * pReturn = NULL;

	if ( pPools && pInterface )
	{
		switch ( pInterface->nType )
		{
            case ENTITY_TYPE_PED:
                pReturn = (CEntity*)(pPools->GetPed( (DWORD *)pInterface ));
                break;
			case ENTITY_TYPE_VEHICLE:
				pReturn = (CEntity*)(pPools->GetVehicle( (DWORD *)pInterface ));
				break;
            case ENTITY_TYPE_OBJECT:
                pReturn = (CEntity*)(pPools->GetObject ( (DWORD *)pInterface ));
                break;
			default:
				break;
		}
	}
	return pReturn;
}

void CPhysicalSA::AttachEntityToEntity( CPhysical& Entity, const CVector& vecPosition, const CVector& vecRotation )
{
	try {
		CPhysicalSA& EntitySA = dynamic_cast < CPhysicalSA& > ( Entity );
		DWORD dwEntityInterface = (DWORD) EntitySA.GetInterface ();

		InternalAttachEntityToEntity( dwEntityInterface, vecPosition, vecRotation );
	} catch ( ... ) {
		DEBUG_TRACE ( "Invalid Entity argument detected");
	}
}


void CPhysicalSA::DetachEntityFromEntity(float fUnkX, float fUnkY, float fUnkZ, bool bUnk)
{
    DEBUG_TRACE("void CPhysicalSA::DetachEntityFromEntity(float fUnkX, float fUnkY, float fUnk, bool bUnk)");
	DWORD dwFunc = FUNC_DetatchEntityFromEntity;
	DWORD dwThis = (DWORD)this->GetInterface();

	// DetachEntityFromEntity appears to crash when there's no entity attached (0x544403, bug 2350)
	// So do a NULL check here
	if (((CPhysicalSAInterface *)this->GetInterface())->pAttachedEntity == NULL ) return;

	_asm
	{
        push    bUnk
        push    fUnkZ
        push    fUnkY
        push    fUnkX
		mov		ecx, dwThis
		call	dwFunc 
	}
}


bool CPhysicalSA::InternalAttachEntityToEntity( DWORD dwEntityInterface, const CVector& vecPosition, const CVector& vecRotation )
{
	float fRX = vecRotation.getX();
	float fRY = vecRotation.getY();
	float fRZ = vecRotation.getZ();
	float fPX = vecPosition.getX();
	float fPY = vecPosition.getY();
	float fPZ = vecPosition.getZ();

	DWORD dwFunc = FUNC_AttachEntityToEntity;
	DWORD dwThis = (DWORD)this->GetInterface();
    DWORD dwReturn = 0;
	_asm
	{
        push    fRZ
        push    fRY
        push    fRX
        push    fPZ
        push    fPY
        push    fPX
        push    dwEntityInterface
		mov		ecx, dwThis
		call	dwFunc
        mov     dwReturn, eax   
	}
    return (dwReturn != NULL);
}


void CPhysicalSA::GetAttachedOffsets( CVector& vecPosition, CVector& vecRotation )
{
    CPhysicalSAInterface *pInterface = (CPhysicalSAInterface *)this->GetInterface();
    if ( pInterface->pAttachedEntity )
    {
		vecPosition = CVectorGTA::unwrap( pInterface->vecAttachedPosition );
		vecRotation = CVectorGTA::unwrap( pInterface->vecAttachedRotation );
    }
}


void CPhysicalSA::SetAttachedOffsets( const CVector& vecPosition, const CVector& vecRotation )
{
    CPhysicalSAInterface * pInterface = (CPhysicalSAInterface *)this->GetInterface();
    if ( pInterface->pAttachedEntity )
    {
        pInterface->vecAttachedPosition = vecPosition;
        pInterface->vecAttachedRotation = vecRotation;
    }
}




/*
void CPhysicalSA::SetMassMultiplier(float fMassMultiplier)
{
	((CPhysicalSAInterface *)this->GetInterface())->MassMultiplier = fMassMultiplier;
}

float CPhysicalSA::GetMassMultiplier()
{
	return ((CPhysicalSAInterface *)this->GetInterface())->MassMultiplier;
}

void CPhysicalSA::SetAirResistance(float fAirResistance)
{
	((CPhysicalSAInterface *)this->GetInterface())->AirResistance = fAirResistance;
}

float CPhysicalSA::GetAirResistance()
{
	return ((CPhysicalSAInterface *)this->GetInterface())->AirResistance;
}

void CPhysicalSA::SetCenterOfMass(CVector * vecCenterOfMass)
{
	memcpy(&((CPhysicalSAInterface *)this->GetInterface())->vecCenterOfMass, vecCenterOfMass, sizeof(CVector));
}

CVector * CPhysicalSA::GetCenterOfMass()
{
	return &((CPhysicalSAInterface *)this->GetInterface())->vecCenterOfMass;
}

bool CPhysicalSA::GetExtraHeavy()
{
	return ((CPhysicalSAInterface *)this->GetInterface())->ExtraHeavy;
}

void CPhysicalSA::SetExtraHeavy(bool bExtraHeavy)
{
	((CPhysicalSAInterface *)this->GetInterface())->ExtraHeavy = bExtraHeavy;
}

bool CPhysicalSA::GetDoGravity()
{
	return ((CPhysicalSAInterface *)this->GetInterface())->DoGravity;
}

void CPhysicalSA::SetDoGravity(bool bDoGravity)
{
	((CPhysicalSAInterface *)this->GetInterface())->DoGravity = bDoGravity;
}

bool CPhysicalSA::GetInfiniteMass()
{
	return ((CPhysicalSAInterface *)this->GetInterface())->InfiniteMass;
}

void CPhysicalSA::SetInfiniteMass(bool bInfiniteMass)
{
	((CPhysicalSAInterface *)this->GetInterface())->InfiniteMass = bInfiniteMass;
}

bool CPhysicalSA::GetPositionFrozen()
{
	return ((CPhysicalSAInterface *)this->GetInterface())->CoorsFrozenByScript;
}

void CPhysicalSA::SetPositionFrozen(bool bPositionFrozen)
{
	((CPhysicalSAInterface *)this->GetInterface())->CoorsFrozenByScript = bPositionFrozen;
}

BYTE CPhysicalSA::GetLastMaterialToHaveBeenStandingOn()
{
	return((CPhysicalSAInterface *)this->GetInterface())->LastMaterialToHaveBeenStandingOn;
}

BYTE CPhysicalSA::GetLevel()
{
	return ((CPhysicalSAInterface *)this->GetInterface())->LivesInThisLevel;
}

void CPhysicalSA::SetLevel(BYTE LivesInThisLevel)
{
	((CPhysicalSAInterface *)this->GetInterface())->LivesInThisLevel = LivesInThisLevel;
}*/