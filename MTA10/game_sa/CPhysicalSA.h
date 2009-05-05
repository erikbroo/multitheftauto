/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CPhysicalSA.h
*  PURPOSE:     Header file for physical object entity base class
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Christian Myhre Lundheim <>
*               Cecill Etheredge <ijsf@gmx.net>
*               Jax <>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_PHYSICAL
#define __CGAMESA_PHYSICAL

#include <game/CPhysical.h>
#include "CEntitySA.h"
#include <CVector.h>

#define FUNC_GetMoveSpeed						0x404460
#define FUNC_GetTurnSpeed						0x470030
#define FUNC_ProcessCollision					0x54DFB0
#define FUNC_AttachEntityToEntity               0x54D570
#define FUNC_DetatchEntityFromEntity            0x5442F0

#define PHYSICAL_MAXNOOFCOLLISIONRECORDS		6

class CPhysicalSAInterface : public CEntitySAInterface // begin +68 (244 bytes total?)
{
public:
/* IMPORTANT: KEEP "pad" in CVehicle UP-TO-DATE if you add something here (or eventually pad someplace else) */
	CVectorGTA * vecVelocity;
	CVectorGTA * vecSpin;
	CVectorGTA * vecUnk3;
	CVectorGTA * vecUnk4;
	CVectorGTA * vecUnk5;
	CVectorGTA * vecUnk6;
    float pad [12];
	float fMass;
    float fTurnMass;

    DWORD dwUnk; // 148
    DWORD dwPhysUnk1; // 152

    float fElasticity; // 156
    float fBuoyancyConstant; // 160
    CVectorGTA * vecCenterOfMass; // 164

    DWORD dwUnk2; // 176
    DWORD * unkCPtrNodeDoubleLink; // 180

    BYTE byUnk: 8; 
    BYTE byCollisionRecords: 8; // 185
    BYTE byUnk2: 8;
    BYTE byUnk3: 8;

    float pad4 [8];

    float fDistanceTravelled; // 212
    float fDamageImpulseMagnitude; //216
    CEntitySAInterface * damageEntity; // 220

    BYTE pad2[28];
	CEntitySAInterface * pAttachedEntity;   // 252
    CVectorGTA vecAttachedPosition;    // 256
    CVectorGTA vecAttachedRotation;    // 268
    BYTE pad3[32];
};

class CPhysicalSA : public virtual CPhysical, public virtual CEntitySA
{
public:
	const CVector	GetMoveSpeed( void );
	const CVector	GetTurnSpeed( void );
	void			SetMoveSpeed( const CVector& vecMoveSpeed );
	void			SetTurnSpeed( const CVector& vecTurnSpeed );

    float			GetMass( void );
    void			SetMass( float fMass );
    float			GetTurnMass( void );
    void			SetTurnMass( float fTurnMass );
    float			GetElasticity( void );
    void			SetElasticity( float fElasticity );
    float			GetBuoyancyConstant( void );
    void			SetBuoyancyConstant( float fBuoyancyConstant );

	void			ProcessCollision( void );

    float			GetDamageImpulseMagnitude( void );
    void			SetDamageImpulseMagnitude( float fMagnitude );
    CEntity*		GetDamageEntity( void );
    void			SetDamageEntity( CEntity* pEntity );
    void			ResetLastDamage( void );

    CEntity*		GetAttachedEntity( void );
    void			AttachEntityToEntity( CPhysical& Entity, const CVector& vecPosition, const CVector& vecRotation );
    void			DetachEntityFromEntity( float fUnkX, float fUnkY, float fUnkZ, bool bUnk );
    void			GetAttachedOffsets( const CVector& vecPosition, const CVector& vecRotation );
    void			SetAttachedOffsets( const CVector& vecPosition, const CVector& vecRotation );

    virtual bool	InternalAttachEntityToEntity( DWORD dwEntityInterface, const CVector& vecPosition, const CVector& vecRotation );
};

#endif