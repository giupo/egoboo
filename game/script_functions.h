#pragma once

//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file script_functions.h
/// @details functions called by scr_run_function()

#include "egoboo_typedef.h"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

struct s_script_state;
struct s_ai_state;
struct s_chr;

//--------------------------------------------------------------------------------------------
// helper functions
//--------------------------------------------------------------------------------------------

Uint8 scr_set_AlertBit( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ClearAlertBit( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TestAlertBit( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Alert( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ClearAlert( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TestAlert( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Bit( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ClearBit( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TestBit( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Bits( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ClearBits( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TestBits( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Spawned( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TimeOut( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AtWaypoint( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AtLastWaypoint( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Attacked( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Bumped( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Ordered( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CalledForHelp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Content( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Killed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetKilled( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ClearWaypoints( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddWaypoint( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_FindPath( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Compass( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_TargetArmorPrice( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Time( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_Content( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_JoinTargetTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToNearbyEnemy( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToTargetLeftHand( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToTargetRightHand( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToWhoeverAttacked( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToWhoeverBumped( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToWhoeverCalledForHelp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToOldTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TurnModeToVelocity( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TurnModeToWatch( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TurnModeToSpin( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_BumpHeight( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasItemID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHoldingItemID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasSkillID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Else( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Run( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Walk( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Sneak( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DoAction( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_KeepAction( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_IssueOrder( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DropWeapons( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetDoAction( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_OpenPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ClosePassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PassageOpen( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_GoPoof( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CostTargetItemID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DoActionOverride( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Healed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SendPlayerMessage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CallForHelp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddIDSZ( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_State( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_State( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetCanOpenStuff( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Grabbed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Dropped( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToWhoeverIsHolding( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DamageTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_XIsLessThanY( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_WeatherTime( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_BumpHeight( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Reaffirmed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_UnkeepAction( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsOnOtherTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsOnHatedTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PressLatchButton( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToTargetOfLeader( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_LeaderKilled( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_BecomeLeader( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ChangeTargetArmor( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_GiveMoneyToTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DropKeys( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_LeaderIsAlive( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsOldTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToLeader( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnCharacter( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_RespawnCharacter( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ChangeTile( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Used( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DropMoney( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_OldTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DetachFromHolder( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasVulnerabilityID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CleanUp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CleanedUp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Sitting( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsHurt( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsAPlayer( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PlaySound( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnParticle( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsAlive( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Stop( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DisaffirmCharacter( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ReaffirmCharacter( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsSelf( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsMale( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsFemale( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToSelf( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToRider( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_AttackTurn( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_DamageType( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_BecomeSpell( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_BecomeSpellbook( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ScoredAHit( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Disaffirmed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TranslateOrder( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToWhoeverWasHit( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToWideEnemy( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Changed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_InWater( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Bored( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TooMuchBaggage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Grogged( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Dazed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasSpecialID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PressTargetLatchButton( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Invisible( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ArmorIs( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_TargetGrogTime( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_TargetDazeTime( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_DamageType( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_WaterLevel( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_EnchantTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_EnchantChild( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TeleportTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetExperience( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_IncreaseAmmo( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_UnkurseTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetTeamExperience( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Unarmed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_RestockTargetAmmoIDAll( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_RestockTargetAmmoIDFirst( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_FlashTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_RedShift( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_GreenShift( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_BlueShift( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Light( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Alpha( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HitFromBehind( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HitFromFront( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HitFromLeft( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HitFromRight( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsOnSameTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_KillTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_UndoEnchant( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_WaterLevel( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CostTargetMana( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasAnyID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_BumpSize( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_NotDropped( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_YIsLessThanX( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_FlyHeight( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Blocked( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsDefending( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsAttacking( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs0( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs1( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs2( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs3( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs4( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs5( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs6( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs7( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ContentIs( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TurnModeToWatchTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIsNot( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_XIsEqualToY( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DebugMessage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_BlackTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SendMessageNear( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HitGround( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_NameIsKnown( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_UsageIsKnown( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HoldingItemID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HoldingRangedWeapon( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HoldingMeleeWeapon( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HoldingShield( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Kursed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsKursed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsDressedUp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_OverWater( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Thrown( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_MakeNameKnown( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_MakeUsageKnown( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StopTargetMovement( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_XY( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_XY( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddXY( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_MakeAmmoKnown( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnAttachedParticle( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnExactParticle( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AccelerateTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_distanceIsMoreThanTurn( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Crushed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_MakeCrushValid( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToLowestTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_NotPutAway( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TakenOut( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AmmoOut( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PlaySoundLooped( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StopSound( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HealSelf( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Equip( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasItemIDEquipped( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_OwnerToTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToOwner( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Frame( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_BreakPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_ReloadTime( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToWideBlahID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PoofTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ChildDoActionOverride( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnPoof( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_SpeedPercent( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_ChildState( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnAttachedSizedParticle( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ChangeArmor( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ShowTimer( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_FacingTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PlaySoundVolume( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnAttachedFacedParticle( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIsOdd( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToDistantEnemy( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Teleport( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetStrength( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetWisdom( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetIntelligence( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetDexterity( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetLife( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetMana( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ShowMap( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ShowYouAreHere( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ShowBlipXY( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HealTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PumpTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CostAmmo( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_MakeSimilarNamesKnown( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnAttachedHolderParticle( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetReloadTime( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_FogLevel( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_FogLevel( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_FogTAD( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_FogBottomLevel( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_FogBottomLevel( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CorrectActionForHand( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsMounted( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SparkleIcon( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_UnsparkleIcon( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_TileXY( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TileXY( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_ShadowSize( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_OrderTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToWhoeverIsInPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CharacterWasABook( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_EnchantBoostValues( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnCharacterXYZ( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnExactCharacterXYZ( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ChangeTargetClass( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PlayFullSound( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnExactChaseParticle( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_CreateOrder( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_OrderSpecialID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_UnkurseTargetInventory( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsSneaking( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DropItems( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_RespawnTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetDoActionSetFrame( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetCanSeeInvisible( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToNearestBlahID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToNearestEnemy( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToNearestFriend( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToNearestLifeform( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_FlashPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_FindTileInPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HeldInLeftHand( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_NotAnItem( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_ChildAmmo( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HitVulnerable( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsFlying( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_IdentifyTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_BeatModule( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_EndModule( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DisableExport( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_EnableExport( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_TargetState( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Equipped( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DropTargetMoney( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_TargetContent( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DropTargetKeys( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_JoinTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetJoinTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ClearMusicPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ClearEndMessage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddEndMessage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PlayMusic( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_MusicPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_MakeCrushInvalid( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StopMusic( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_FlashVariable( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AccelerateUp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_FlashVariableHeight( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_DamageTime( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs8( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs9( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs10( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs11( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs12( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs13( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs14( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_StateIs15( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsAMount( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsAPlatform( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddStat( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DisenchantTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DisenchantAll( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_VolumeNearestTeammate( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddShopPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetPayForArmor( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_JoinEvilTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_JoinNullTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_JoinGoodTeam( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PitsKill( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToPassageID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_MakeNameUnknown( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnExactParticleEndSpawn( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnPoofSpeedSpacingDamage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_GoodTeamExperience( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DoNothing( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_GrogTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DazeTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_EnableRespawn( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DisableRespawn( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DispelTargetEnchantID( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_HolderBlocked( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasNotFullMana( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_EnableListenSkill( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToLastItemUsed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_FollowLink( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_OperatorIsLinux( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsAWeapon( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SomeoneIsStealing( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsASpell( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_Backstabbed( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_get_TargetDamageType( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddQuest( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_BeatQuestAllPlayers( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetHasQuest( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_QuestLevel( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddQuestAllPlayers( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AddBlipAllEnemies( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_PitsFall( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsOwner( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_End( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TakePicture( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Speech( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_MoveSpeech( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_SecondMoveSpeech( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_AttackSpeech( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_AssistSpeech( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TerrainSpeech( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_SelectSpeech( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_OperatorIsMacintosh( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_ModuleHasIDSZ( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_MorphToTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetManaFlow( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetManaReturn( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_Money( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetCanSeeKurses( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_SpawnAttachedCharacter( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_KurseTarget( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_ChildContent( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToChild( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_DamageThreshold( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_AccelerateTargetUp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetAmmo( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_EnableInvictus( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DisableInvictus( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetDamageSelf( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetSize( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_DrawBillboard( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToBlahInPassage( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_TargetIsFacingSelf( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_LevelUp( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_add_TargetSkill( struct s_script_state * pstate, struct s_ai_state * pself );
Uint8 scr_set_TargetToNearbyMeleeWeapon( struct s_script_state * pstate, struct s_ai_state * pself );
