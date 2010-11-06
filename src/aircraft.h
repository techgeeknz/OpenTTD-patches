/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file aircraft.h Base for aircraft. */

#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include "station_map.h"
#include "vehicle_base.h"

struct Aircraft;

/** An aircraft can be one of those types. */
enum AircraftSubType {
	AIR_HELICOPTER = 0, ///< an helicopter
	AIR_AIRCRAFT   = 2, ///< an airplane
	AIR_SHADOW     = 4, ///< shadow of the aircraft
	AIR_ROTOR      = 6  ///< rotor of an helicopter
};


/**
 * Handle Aircraft specific tasks when an Aircraft enters a hangar
 * @param *v Vehicle that enters the hangar
 */
void HandleAircraftEnterHangar(Aircraft *v);

/**
 * Get the size of the sprite of an aircraft sprite heading west (used for lists)
 * @param engine The engine to get the sprite from
 * @param width The width of the sprite
 * @param height The height of the sprite
 */
void GetAircraftSpriteSize(EngineID engine, uint &width, uint &height);

/**
 * Updates the status of the Aircraft heading or in the station
 * @param st Station been updated
 */
void UpdateAirplanesOnNewStation(const Station *st);

/**
 * Update cached values of an aircraft.
 * Currently caches callback 36 max speed.
 * @param v Vehicle
 */
void UpdateAircraftCache(Aircraft *v);

void AircraftLeaveHangar(Aircraft *v);
void AircraftNextAirportPos_and_Order(Aircraft *v);
void SetAircraftPosition(Aircraft *v, int x, int y, int z);
byte GetAircraftFlyingAltitude(const Aircraft *v);

/** Cached oftenly queried (NewGRF) values */
struct AircraftCache {
	uint16 cached_max_speed; ///< Cached maximum speed of the aircraft.
};

/**
 * Aircraft, helicopters, rotors and their shadows belong to this class.
 */
struct Aircraft : public SpecializedVehicle<Aircraft, VEH_AIRCRAFT> {
	AircraftCache acache;          ///< Cache of often used calculated values

	uint16 crashed_counter;        ///< Timer for handling crash animations.
	byte pos;                      ///< Next desired position of the aircraft.
	byte previous_pos;             ///< Previous desired position of the aircraft.
	StationID targetairport;       ///< Airport to go to next.
	byte state;                    ///< State of the airport. @see AirportMovementStates
	DirectionByte last_direction;
	byte number_consecutive_turns; ///< Protection to prevent the aircraft of making a lot of turns in order to reach a specific point.
	byte turn_counter;             ///< Ticks between each turn to prevent > 45 degree turns.

	/** We don't want GCC to zero our struct! It already is zeroed and has an index! */
	Aircraft() : SpecializedVehicle<Aircraft, VEH_AIRCRAFT>() {}
	/** We want to 'destruct' the right class. */
	virtual ~Aircraft() { this->PreDestructor(); }

	const char *GetTypeString() const { return "aircraft"; }
	void MarkDirty();
	void UpdateDeltaXY(Direction direction);
	ExpensesType GetExpenseType(bool income) const { return income ? EXPENSES_AIRCRAFT_INC : EXPENSES_AIRCRAFT_RUN; }
	bool IsPrimaryVehicle() const                  { return this->IsNormalAircraft(); }
	SpriteID GetImage(Direction direction) const;
	int GetDisplaySpeed() const    { return this->cur_speed; }
	int GetDisplayMaxSpeed() const { return this->acache.cached_max_speed; }
	int GetSpeedOldUnits() const   { return this->acache.cached_max_speed * 10 / 128; }
	Money GetRunningCost() const;
	bool IsInDepot() const { return (this->vehstatus & VS_HIDDEN) != 0 && IsHangarTile(this->tile); }
	bool Tick();
	void OnNewDay();
	uint Crash(bool flooded = false);
	TileIndex GetOrderStationLocation(StationID station);
	bool FindClosestDepot(TileIndex *location, DestinationID *destination, bool *reverse);

	/**
	 * Check if the aircraft type is a normal flying device; eg
	 * not a rotor or a shadow
	 * @return Returns true if the aircraft is a helicopter/airplane and
	 * false if it is a shadow or a rotor
	 */
	FORCEINLINE bool IsNormalAircraft() const
	{
		/* To be fully correct the commented out functionality is the proper one,
		 * but since value can only be 0 or 2, it is sufficient to only check <= 2
		 * return (this->subtype == AIR_HELICOPTER) || (this->subtype == AIR_AIRCRAFT); */
		return this->subtype <= AIR_AIRCRAFT;
	}
};

/**
 * Macro for iterating over all aircrafts.
 */
#define FOR_ALL_AIRCRAFT(var) FOR_ALL_VEHICLES_OF_TYPE(Aircraft, var)

SpriteID GetRotorImage(const Aircraft *v);

Station *GetTargetAirportIfValid(const Aircraft *v);

#endif /* AIRCRAFT_H */
