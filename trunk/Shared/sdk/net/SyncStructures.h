/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/sdk/net/SyncStructures.h
*  PURPOSE:     Structures used for syncing stuff through the network.
*  DEVELOPERS:  Alberto Alonso <rydencillo@gmail.com>
*               Stanislav Bobrov <lil_toady@hotmail.com>
*               
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#pragma once

#include <CVector.h>
#include <net/bitstream.h>
#include <ieee754.h>

#pragma pack(push)
#pragma pack(1)


//////////////////////////////////////////
//                                      //
//              Data types              //
//                                      //
//////////////////////////////////////////
template < unsigned int integerBits, unsigned int fractionalBits >
struct SFloatSync : public ISyncStructure
{
    bool Read ( NetBitStreamInterface& bitStream )
    {
        SFixedPointNumber num;
        if ( bitStream.ReadBits ( &num, integerBits + fractionalBits ) )
        {
            data.fValue = (float)( (double)num.iValue / ( 1 << fractionalBits ) );
            return true;
        }
        return false;
    }

    void Write ( NetBitStreamInterface& bitStream ) const
    {
        struct
        {
            int iMin : integerBits;
            int iMax : integerBits;
        } limits;
        limits.iMax = ( 1 << ( integerBits - 1 ) ) - 1;
        limits.iMin = limits.iMax + 1;

        IEEE754_DP dValue ( data.fValue );
        assert ( !dValue.isnan () );

        if ( dValue > limits.iMax ) dValue = (double)limits.iMax;
        else if ( dValue < limits.iMin ) dValue = (double)limits.iMin;

        SFixedPointNumber num;
        num.iValue = (int)( dValue * (double)( 1 << fractionalBits ));

        bitStream.WriteBits ( &num, integerBits + fractionalBits );
    }

    struct
    {
        float fValue;
    } data;

private:
    struct SFixedPointNumber
    {
        int iValue : integerBits + fractionalBits;
    };
};



//////////////////////////////////////////
//                                      //
//               Position               //
//                                      //
//////////////////////////////////////////
struct SPositionSync : public ISyncStructure
{
    SPositionSync ( bool bUseFloats = false ) : m_bUseFloats ( bUseFloats ) {}

    bool Read ( NetBitStreamInterface& bitStream )
    {
        if ( m_bUseFloats )
        {
            return bitStream.Read ( data.vecPosition.fX ) &&
                   bitStream.Read ( data.vecPosition.fY ) &&
                   bitStream.Read ( data.vecPosition.fZ );
        }
        else
        {
            SFloatSync < 14, 10 > x, y, z;

            if ( bitStream.Read ( &x ) && bitStream.Read ( &y ) && bitStream.Read ( &z ) )
            {
                data.vecPosition.fX = x.data.fValue;
                data.vecPosition.fY = y.data.fValue;
                data.vecPosition.fZ = z.data.fValue;
                return true;
            }
        }

        return false;
    }

    void Write ( NetBitStreamInterface& bitStream ) const
    {
        if ( m_bUseFloats )
        {
            bitStream.Write ( data.vecPosition.fX );
            bitStream.Write ( data.vecPosition.fY );
            bitStream.Write ( data.vecPosition.fZ );
        }
        else
        {
            SFloatSync < 14, 10 > x, y, z;
            x.data.fValue = data.vecPosition.fX;
            y.data.fValue = data.vecPosition.fY;
            z.data.fValue = data.vecPosition.fZ;

            bitStream.Write ( &x );
            bitStream.Write ( &y );
            bitStream.Write ( &z );
        }
    }

    struct
    {
        CVector vecPosition;
    } data;

private:
    bool m_bUseFloats;
};



//////////////////////////////////////////
//                                      //
//               Velocity               //
//                                      //
//////////////////////////////////////////
struct SVelocitySync : public ISyncStructure
{
    bool Read ( NetBitStreamInterface& bitStream )
    {
        if ( !bitStream.ReadBit () )
        {
            data.vecVelocity.fX = data.vecVelocity.fY = data.vecVelocity.fZ = 0.0f;
            return true;
        }
        else
        {
            float fModule;
            if ( bitStream.Read ( fModule ) )
            {
                if ( bitStream.ReadNormVector ( data.vecVelocity.fX, data.vecVelocity.fY, data.vecVelocity.fZ ) )
                {
                    data.vecVelocity = data.vecVelocity * fModule;
                    return true;
                }
            }
        }
        return false;
    }

    void Write ( NetBitStreamInterface& bitStream ) const
    {
        CVector vecVelocity = data.vecVelocity;

        float fModulus = vecVelocity.Normalize ();
        if ( fModulus == 0.0f )
            bitStream.WriteBit ( false );
        else
        {
            bitStream.WriteBit ( true );
            bitStream.Write ( fModulus );
            bitStream.WriteNormVector ( vecVelocity.fX, vecVelocity.fY, vecVelocity.fZ );
        }
    }

    struct
    {
        CVector vecVelocity;
    } data;
};



//////////////////////////////////////////
//                                      //
//           Player pure-sync           //
//                                      //
//////////////////////////////////////////
struct SPlayerPuresyncFlags : public ISyncStructure
{
    enum { BITCOUNT = 11 };

    bool Read ( NetBitStreamInterface& bitStream )
    {
        return bitStream.ReadBits ( &data, BITCOUNT );
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        bitStream.WriteBits ( &data, BITCOUNT );
    }

    struct
    {
        bool bIsInWater : 1;
        bool bIsOnGround : 1;
        bool bHasJetPack : 1;
        bool bIsDucked : 1;
        bool bWearsGoogles : 1;
        bool bHasContact : 1;
        bool bIsChoking : 1;
        bool bAkimboTargetUp : 1;
        bool bIsOnFire : 1;
        bool bHasAWeapon : 1;
        bool bSyncingVelocity : 1;
    } data;
};


struct SPedRotationSync : public ISyncStructure
{
    bool Read ( NetBitStreamInterface& bitStream )
    {
        unsigned short sValue;
        if ( bitStream.Read ( sValue ) )
        {
            data.fRotation = static_cast < float > ( sValue ) * 3.14159265f / 16200.0f;
            return true;
        }
        return false;
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        float fRotation = data.fRotation;
        short sValue = static_cast < short > ( fRotation * 16200.0f / 3.14159265f );
        bitStream.Write ( sValue );
    }

    struct
    {
        float fRotation;
    } data;
};



//////////////////////////////////////////
//                                      //
//       Unoccupied vehicle sync        //
//                                      //
//////////////////////////////////////////
struct SUnoccupiedVehicleSync : public ISyncStructure
{
    SUnoccupiedVehicleSync () { *((char *)&data) = 0; }

    bool Read ( NetBitStreamInterface& bitStream )
    {
        if ( bitStream.ReadCompressed ( data.vehicleID ) &&
             bitStream.Read ( data.ucTimeContext ) &&
             bitStream.ReadBits ( &data, 8 ) )
        {
            if ( data.bSyncPosition )
            {
                SPositionSync pos;
                bitStream.Read ( &pos );
                data.vecPosition = pos.data.vecPosition;
            }

            if ( data.bSyncRotation )
            {
                bitStream.Read ( data.vecRotation.fX );
                bitStream.Read ( data.vecRotation.fY );
                bitStream.Read ( data.vecRotation.fZ );
            }

            if ( data.bSyncVelocity )
            {
                SVelocitySync velocity;
                bitStream.Read ( &velocity );
                data.vecVelocity = velocity.data.vecVelocity;
            }

            if ( data.bSyncTurnVelocity )
            {
                bitStream.Read ( data.vecTurnVelocity.fX );
                bitStream.Read ( data.vecTurnVelocity.fY );
                bitStream.Read ( data.vecTurnVelocity.fZ );
            }

            if ( data.bSyncHealth )
            {
                bitStream.Read ( data.fHealth );
            }

            if ( data.bSyncTrailer )
            {
                bitStream.ReadCompressed ( data.trailer );
            }

            return true;
        }
        return false;
    }

    void Write ( NetBitStreamInterface& bitStream ) const
    {
        bitStream.WriteCompressed ( data.vehicleID );
        bitStream.Write ( data.ucTimeContext );
        bitStream.WriteBits ( &data, 8 );

        if ( data.bSyncPosition )
        {
            SPositionSync pos;
            pos.data.vecPosition = data.vecPosition;
            bitStream.Write ( &pos );
        }

        if ( data.bSyncRotation )
        {
            bitStream.Write ( data.vecRotation.fX );
            bitStream.Write ( data.vecRotation.fY );
            bitStream.Write ( data.vecRotation.fZ );
        }

        if ( data.bSyncVelocity )
        {
            SVelocitySync velocity;
            velocity.data.vecVelocity = data.vecVelocity;
            bitStream.Write ( &velocity );
        }

        if ( data.bSyncTurnVelocity )
        {
            bitStream.Write ( data.vecTurnVelocity.fX );
            bitStream.Write ( data.vecTurnVelocity.fY );
            bitStream.Write ( data.vecTurnVelocity.fZ );
        }

        if ( data.bSyncHealth )
        {
            bitStream.Write ( data.fHealth );
        }

        if ( data.bSyncTrailer )
        {
            bitStream.WriteCompressed ( data.trailer );
        }
    }

    struct
    {
        bool bSyncPosition : 1;
        bool bSyncRotation : 1;
        bool bSyncVelocity : 1;
        bool bSyncTurnVelocity : 1;
        bool bSyncHealth : 1;
        bool bSyncTrailer : 1;
        bool bEngineOn : 1;
        bool bDerailed : 1;
        CVector vecPosition;
        CVector vecRotation;
        CVector vecVelocity;
        CVector vecTurnVelocity;
        float fHealth;
        ElementID trailer;

        ElementID vehicleID;
        unsigned char ucTimeContext;
    } data;
};



//////////////////////////////////////////
//                                      //
//               Keysync                //
//                                      //
//////////////////////////////////////////
struct SKeysyncFlags : public ISyncStructure
{
    enum { BITCOUNT = 3 };

    bool Read ( NetBitStreamInterface& bitStream )
    {
        return bitStream.ReadBits ( &data, BITCOUNT );
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        bitStream.WriteBits ( &data, BITCOUNT );
    }

    struct
    {
        bool bIsDucked : 1;
        bool bIsChoking : 1;
        bool bAkimboTargetUp : 1;
    } data;
};

struct SFullKeysyncSync : public ISyncStructure
{
    // Stick values vary from -128 to 128, but char range is from -128 to 127, so we stretch
    // the stick value to the range from -127 to 127 to make it fit in a char (byte) and save
    // one byte of bandwidth per stick.
    bool Read ( NetBitStreamInterface& bitStream )
    {
        bool bState;
        char cLeftStickX;
        char cLeftStickY;

        if ( ( bState = bitStream.ReadBits ( &data, 8 ) ) )
        {
            if ( ( bState = bitStream.Read ( cLeftStickX ) ) )
            {
                data.sLeftStickX = static_cast < short > ( (float)cLeftStickX * 128.0f/127.0f );
                if ( ( bState = bitStream.Read ( cLeftStickY ) ) )
                    data.sLeftStickY = static_cast < short > ( (float)cLeftStickY * 128.0f/127.0f );
            }
        }

        return bState;
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        bitStream.WriteBits ( &data, 8 );
        char cLeftStickX = static_cast < char > ( (float)data.sLeftStickX * 127.0f/128.0f );
        bitStream.Write ( cLeftStickX );
        char cLeftStickY = static_cast < char > ( (float)data.sLeftStickY * 127.0f/128.0f );
        bitStream.Write ( cLeftStickY );
    }

    struct
    {
        bool bLeftShoulder1 : 1;
        bool bRightShoulder1 : 1;
        bool bButtonSquare : 1;
        bool bButtonCross : 1;
        bool bButtonCircle : 1;
        bool bButtonTriangle : 1;
        bool bShockButtonL : 1;
        bool bPedWalk : 1;
        short sLeftStickX;
        short sLeftStickY;
    } data;
};

struct SSmallKeysyncSync : public ISyncStructure
{
    // Stick values vary from -128 to 128, but char range is from -128 to 127, so we stretch
    // the stick value to the range from -127 to 127 to make it fit in a char (byte) and save
    // one byte of bandwidth per stick.
    bool Read ( NetBitStreamInterface& bitStream )
    {
        bool bState;
        char cLeftStickX;
        char cLeftStickY;

        if ( ( bState = bitStream.ReadBits ( &data, 10 ) ) )
        {
            if ( data.bLeftStickXChanged && ( bState = bitStream.Read ( cLeftStickX ) ) )
                data.sLeftStickX = static_cast < short > ( (float)cLeftStickX * 128.0f/127.0f );
            if ( bState && data.bLeftStickYChanged && ( bState = bitStream.Read ( cLeftStickY ) ) )
                data.sLeftStickY = static_cast < short > ( (float)cLeftStickY * 128.0f/127.0f );
        }

        return bState;
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        bitStream.WriteBits ( &data, 10 );
        if ( data.bLeftStickXChanged )
        {
            char cLeftStickX = static_cast < char > ( (float)data.sLeftStickX * 127.0f/128.0f );
            bitStream.Write ( cLeftStickX );
        }
        if ( data.bLeftStickYChanged )
        {
            char cLeftStickY = static_cast < char > ( (float)data.sLeftStickY * 127.0f/128.0f );
            bitStream.Write ( cLeftStickY );
        }
    }

    struct
    {
        bool bLeftShoulder1 : 1;
        bool bRightShoulder1 : 1;
        bool bButtonSquare : 1;
        bool bButtonCross : 1;
        bool bButtonCircle : 1;
        bool bButtonTriangle : 1;
        bool bShockButtonL : 1;
        bool bPedWalk : 1;
        bool bLeftStickXChanged : 1;
        bool bLeftStickYChanged : 1;
        short sLeftStickX;
        short sLeftStickY;
    } data;
};

struct SVehicleSpecific : public ISyncStructure
{
    bool Read ( NetBitStreamInterface& bitStream )
    {
        unsigned short usHorizontal, usVertical;
        if ( bitStream.Read ( usHorizontal ) && bitStream.Read ( usVertical ) )
        {
            data.fTurretX = static_cast < float > ( usHorizontal ) / ( 65535.0f / 360.0f );
            data.fTurretY = static_cast < float > ( usVertical ) / ( 65535.0f / 360.0f );

            return true;
        }
        return false;
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        // Convert to shorts to save 4 bytes (multiply for precision on how many rounds can fit in a ushort)
        unsigned short usHorizontal = static_cast < unsigned short > ( data.fTurretX * ( 65535.0f / 360.0f ) ),
                       usVertical = static_cast < unsigned short > ( data.fTurretY * ( 65535.0f / 360.0f ) );

        bitStream.Write ( usHorizontal );
        bitStream.Write ( usVertical );
    }

    struct
    {
        float fTurretX;
        float fTurretY;
    } data;
};



//////////////////////////////////////////
//                                      //
//             Weapon sync              //
//                                      //
//////////////////////////////////////////
struct SWeaponSlotSync : public ISyncStructure
{
    enum { BITCOUNT = 4 };

    bool Read ( NetBitStreamInterface& bitStream )
    {
        return bitStream.ReadBits ( &data, BITCOUNT );
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        bitStream.WriteBits ( &data, BITCOUNT );
    }

    struct
    {
        unsigned int uiSlot : 4;
    } data;
};

struct SWeaponTypeSync : public ISyncStructure
{
    enum { BITCOUNT = 6 };

    bool Read ( NetBitStreamInterface& bitStream )
    {
        return bitStream.ReadBits ( &data, BITCOUNT );
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        bitStream.WriteBits ( &data, BITCOUNT );
    }

    struct
    {
        unsigned int uiWeaponType : 6;
    } data;
};

struct SAmmoInClipSync : public virtual ISyncStructure
{
private:
    unsigned char m_usBitCount;

public:	
    SAmmoInClipSync ( unsigned short usAmmoInClip, unsigned char usBitCount = 16 )
    {
        data.usAmmoInClip = usAmmoInClip;
        m_usBitCount = usBitCount;
    }

    bool Read ( NetBitStreamInterface& bitStream )
    {
        return bitStream.ReadBits ( &data, m_usBitCount );
    }

    void Write ( NetBitStreamInterface& bitStream ) const
    {
        bitStream.WriteBits ( &data, m_usBitCount );
    }
    
    unsigned short GetAmmoInClip () const
    {
        return data.usAmmoInClip;
    }

    struct
    {
        unsigned short usAmmoInClip;
    } data;
};

struct SWeaponAmmoSync : public ISyncStructure
{
    SWeaponAmmoSync ( unsigned char ucWeaponType, bool bSyncTotalAmmo = true, bool bSyncAmmoInClip = true )
    : m_ucWeaponType ( ucWeaponType ), m_bSyncTotalAmmo ( bSyncTotalAmmo ), m_bSyncAmmoInClip ( bSyncAmmoInClip )
    {
    }

    bool Read ( NetBitStreamInterface& bitStream )
    {
        bool bStatus = true;
        if ( m_bSyncTotalAmmo )
            bStatus = bitStream.ReadCompressed ( data.usTotalAmmo );

        if ( m_bSyncAmmoInClip && bStatus == true )
        {
            unsigned int usBitCount = GetAmmoInClipSyncBitCount ();
            if ( usBitCount != 0 )
            {
                SAmmoInClipSync ammoInClipSync = SAmmoInClipSync( data.usAmmoInClip, usBitCount );
                bStatus = bitStream.Read ( &ammoInClipSync );
                if ( bStatus )
                    data.usAmmoInClip = ammoInClipSync.GetAmmoInClip ();
                else
                    data.usAmmoInClip = 0;
            }
            else
                bStatus = false;
        }

        return bStatus;
    }

    void Write ( NetBitStreamInterface& bitStream ) const
    {
        if ( m_bSyncTotalAmmo )
            bitStream.WriteCompressed ( data.usTotalAmmo );
        if ( m_bSyncAmmoInClip )
        {
            unsigned int usBitCount = GetAmmoInClipSyncBitCount ();
            if ( usBitCount != 0 )
            {
                SAmmoInClipSync ammoInClipSync = SAmmoInClipSync( data.usAmmoInClip, usBitCount );
                bitStream.Write ( &ammoInClipSync );
            }
        }
    }

    struct
    {
        unsigned short usTotalAmmo;
        unsigned short usAmmoInClip;
    } data;

private:
    unsigned char   m_ucWeaponType;
    bool            m_bSyncTotalAmmo;
    bool            m_bSyncAmmoInClip;

    unsigned int GetAmmoInClipSyncBitCount ( ) const
    {
        switch ( m_ucWeaponType )
        {
            case 22: return 6; // Pistol
            case 23: return 5; // Silenced
            case 24: return 3; // Deagle
            case 25: return 1; // Shotgun
            case 26: return 3; // Sawnoff
            case 27: return 3; // Spas12
            case 28: return 7; // Uzi
            case 29: return 5; // Mp5
            case 32: return 7; // Tec9
            case 30: return 5; // Ak47
            case 31: return 6; // M4
            case 33: return 1; // Rifle
            case 34: return 1; // Sniper
            case 35: return 1; // RLauncher
            case 36: return 1; // RPG
            case 37: return 6; // FThrower
            case 38: return 9; // Minigun
            case 16: return 1; // Grenade
            case 17: return 1; // TearGas
            case 18: return 1; // Molotov
            case 39: return 1; // Satchel
            case 41: return 9; // Spraycan
            case 42: return 9; // FireExt
            case 43: return 6; // Camera
            default: return 0; // Melee
        }
    }

};

struct SWeaponAimSync : public ISyncStructure
{
    SWeaponAimSync ( float fWeaponRange = 0.0f, bool bFull = true ) : m_fWeaponRange ( fWeaponRange ), m_bFull ( bFull ) {}

    bool Read ( NetBitStreamInterface& bitStream )
    {
        bool bStatus = true;
        CVector vecDirection;

        short sArmY;
        if ( bStatus = bitStream.Read ( sArmY ) )
        {
            data.fArm = ( static_cast < float > ( sArmY ) * 3.14159265f / 180 ) / 90.0f;
        }

        if ( m_bFull && bStatus )
        {
            if (( bStatus = bitStream.Read ( data.vecOrigin.fX ) ))
                if (( bStatus = bitStream.Read ( data.vecOrigin.fY ) ))
                    if (( bStatus = bitStream.Read ( data.vecOrigin.fZ ) ))
                        bStatus = bitStream.ReadNormVector ( vecDirection.fX, vecDirection.fZ, vecDirection.fY );

            if ( bStatus )
                data.vecTarget = data.vecOrigin + ( vecDirection * m_fWeaponRange );
            else
                data.vecOrigin = data.vecTarget = CVector ( );
        }

        return bStatus;
    }

    void Write ( NetBitStreamInterface& bitStream ) const
    {
        // Write arm direction (We only sync one arm, Y axis for on foot sync and X axis for driveby)
        short sArm = static_cast < short > ( data.fArm * 90.0f * 180.0f / 3.14159265f );
	    bitStream.Write ( sArm );

        if ( m_bFull )
        {
            // Write the origin of the bullets
            bitStream.Write ( data.vecOrigin.fX );
            bitStream.Write ( data.vecOrigin.fY );
            bitStream.Write ( data.vecOrigin.fZ );

            // Get the direction of the bullets
            CVector vecDirection = data.vecTarget - data.vecOrigin;
            vecDirection.Normalize ();

            // Write the normalized vector
            bitStream.WriteNormVector ( vecDirection.fX, vecDirection.fZ, vecDirection.fY );
        }
    }

    bool isFull ()
    {
        return m_bFull;
    }

    struct
    {
        float fArm;
        CVector vecOrigin;
        CVector vecTarget;
    } data;

private:
    float   m_fWeaponRange;
    bool    m_bFull;
};



//////////////////////////////////////////
//                                      //
//                Others                //
//                                      //
//////////////////////////////////////////
struct SBodypartSync : public ISyncStructure
{
    enum { BITCOUNT = 3 };

    bool Read ( NetBitStreamInterface& bitStream )
    {
        bool bStatus = bitStream.ReadBits ( &data, BITCOUNT );
        if ( bStatus )
            data.uiBodypart += 3;
        else
            data.uiBodypart = 0;
        return bStatus;
    }
    void Write ( NetBitStreamInterface& bitStream ) const
    {
        struct
        {
            unsigned int uiBodypart : 3;
        } privateData;

        // Bodyparts go from 3 to 9, so substracting 3 from the value
        // and then restoring it will save 1 bit.
        privateData.uiBodypart = data.uiBodypart - 3;
        bitStream.WriteBits ( &privateData, BITCOUNT );
    }

    struct
    {
        unsigned int uiBodypart;
    } data;
};


#pragma pack(pop)