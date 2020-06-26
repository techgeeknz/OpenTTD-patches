/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file viewport_type.h Types related to viewports. */

#ifndef VIEWPORT_TYPE_H
#define VIEWPORT_TYPE_H

#include "zoom_type.h"
#include "strings_type.h"
#include "table/strings.h"

#include <vector>

class LinkGraphOverlay;

enum ViewportMapType {
	VPMT_BEGIN = 0,
	VPMT_VEGETATION = 0,
	VPMT_OWNER,
	VPMT_INDUSTRY,
	VPMT_END,

	VPMT_MIN = VPMT_VEGETATION,
	VPMT_MAX = VPMT_INDUSTRY,
};

struct ViewPortMapDrawVehiclesCache {
	uint64 done_hash_bits[64];
	std::vector<bool> vehicle_pixels;
};

/**
 * Data structure for viewport, display of a part of the world
 */
struct ViewPort {
	int left;    ///< Screen coordinate left edge of the viewport
	int top;     ///< Screen coordinate top edge of the viewport
	int width;   ///< Screen width of the viewport
	int height;  ///< Screen height of the viewport

	int virtual_left;    ///< Virtual left coordinate
	int virtual_top;     ///< Virtual top coordinate
	int virtual_width;   ///< width << zoom
	int virtual_height;  ///< height << zoom

	ZoomLevel zoom;      ///< The zoom level of the viewport.
	ViewportMapType map_type;  ///< Rendering type

	std::shared_ptr<LinkGraphOverlay> overlay;

	std::vector<bool> dirty_blocks;
	uint dirty_blocks_per_column;
	uint dirty_blocks_per_row;
	uint8 dirty_block_left_margin;
	bool is_dirty = false;
	bool is_drawn = false;
	ViewPortMapDrawVehiclesCache map_draw_vehicles_cache;

	uint GetDirtyBlockWidthShift() const { return this->GetDirtyBlockShift(); }
	uint GetDirtyBlockHeightShift() const { return this->GetDirtyBlockShift(); }
	uint GetDirtyBlockWidth() const { return 1 << this->GetDirtyBlockWidthShift(); }
	uint GetDirtyBlockHeight() const { return 1 << this->GetDirtyBlockHeightShift(); }

	void ClearDirty()
	{
		if (this->is_dirty) {
			this->dirty_blocks.assign(this->dirty_blocks.size(), false);
			this->is_dirty = false;
		}
		this->is_drawn = false;
	}

private:
	uint GetDirtyBlockShift() const
	{
		if (this->zoom >= ZOOM_LVL_DRAW_MAP) return 3;
		if (this->zoom >= ZOOM_LVL_OUT_8X) return 4;
		return 7 - this->zoom;
	}
};

/** Margins for the viewport sign */
enum ViewportSignMargin {
	VPSM_LEFT   = 1, ///< Left margin
	VPSM_RIGHT  = 1, ///< Right margin
	VPSM_TOP    = 1, ///< Top margin
	VPSM_BOTTOM = 1, ///< Bottom margin
};

/** Location information about a sign as seen on the viewport */
struct ViewportSign {
	int32 center;        ///< The center position of the sign
	int32 top;           ///< The top of the sign
	uint16 width_normal; ///< The width when not zoomed out (normal font)
	uint16 width_small;  ///< The width when zoomed out (small font)

	void UpdatePosition(ZoomLevel maxzoom, int center, int top, StringID str, StringID str_small = STR_NULL);
	void MarkDirty(ZoomLevel maxzoom) const;
};

/** Specialised ViewportSign that tracks whether it is valid for entering into a Kdtree */
struct TrackedViewportSign : ViewportSign {
	bool kdtree_valid; ///< Are the sign data valid for use with the _viewport_sign_kdtree?

	/**
	 * Update the position of the viewport sign.
	 * Note that this function hides the base class function.
	 */
	void UpdatePosition(ZoomLevel maxzoom, int center, int top, StringID str, StringID str_small = STR_NULL)
	{
		this->kdtree_valid = true;
		this->ViewportSign::UpdatePosition(maxzoom, center, top, str, str_small);
	}


	TrackedViewportSign() : kdtree_valid{ false }
	{
	}
};

/**
 * Directions of zooming.
 * @see DoZoomInOutWindow
 */
enum ZoomStateChange {
	ZOOM_IN   = 0, ///< Zoom in (get more detailed view).
	ZOOM_OUT  = 1, ///< Zoom out (get helicopter view).
	ZOOM_NONE = 2, ///< Hack, used to update the button status.
};

/**
 * Some values for constructing bounding boxes (BB). The Z positions under bridges are:
 * z=0..5  Everything that can be built under low bridges.
 * z=6     reserved, currently unused.
 * z=7     Z separator between bridge/tunnel and the things under/above it.
 */
static const uint BB_HEIGHT_UNDER_BRIDGE = 6; ///< Everything that can be built under low bridges, must not exceed this Z height.
static const uint BB_Z_SEPARATOR         = 7; ///< Separates the bridge/tunnel from the things under/above it.

/** Viewport place method (type of highlighted area and placed objects) */
enum ViewportPlaceMethod {
	VPM_X_OR_Y          =    0, ///< drag in X or Y direction
	VPM_FIX_X           =    1, ///< drag only in X axis
	VPM_FIX_Y           =    2, ///< drag only in Y axis
	VPM_X_AND_Y         =    3, ///< area of land in X and Y directions
	VPM_X_AND_Y_LIMITED =    4, ///< area of land of limited size
	VPM_FIX_HORIZONTAL  =    5, ///< drag only in horizontal direction
	VPM_FIX_VERTICAL    =    6, ///< drag only in vertical direction
	VPM_X_LIMITED       =    7, ///< Drag only in X axis with limited size
	VPM_Y_LIMITED       =    8, ///< Drag only in Y axis with limited size
	VPM_A_B_LINE        =    9, ///< Drag a line from tile A to tile B
	VPM_RAILDIRS        = 0x40, ///< all rail directions
	VPM_SIGNALDIRS      = 0x80, ///< similar to VMP_RAILDIRS, but with different cursor
};
DECLARE_ENUM_AS_BIT_SET(ViewportPlaceMethod)

/**
 * Drag and drop selection process, or, what to do with an area of land when
 * you've selected it.
 */
enum ViewportDragDropSelectionProcess {
	DDSP_DEMOLISH_AREA,        ///< Clear area
	DDSP_RAISE_AND_LEVEL_AREA, ///< Raise / level area
	DDSP_LOWER_AND_LEVEL_AREA, ///< Lower / level area
	DDSP_LEVEL_AREA,           ///< Level area
	DDSP_CREATE_DESERT,        ///< Fill area with desert
	DDSP_CREATE_ROCKS,         ///< Fill area with rocks
	DDSP_CREATE_WATER,         ///< Create a canal
	DDSP_CREATE_RIVER,         ///< Create rivers
	DDSP_PLANT_TREES,          ///< Plant trees
	DDSP_BUILD_BRIDGE,         ///< Bridge placement
	DDSP_MEASURE,              ///< Measurement tool
	DDSP_DRAW_PLANLINE,        ///< Draw a line for a plan
	DDSP_BUY_LAND,             ///< Purchase land

	/* Rail specific actions */
	DDSP_PLACE_RAIL,           ///< Rail placement
	DDSP_BUILD_SIGNALS,        ///< Signal placement
	DDSP_BUILD_STATION,        ///< Station placement
	DDSP_REMOVE_STATION,       ///< Station removal
	DDSP_CONVERT_RAIL,         ///< Rail conversion

	/* Road specific actions */
	DDSP_PLACE_ROAD_X_DIR,     ///< Road placement (X axis)
	DDSP_PLACE_ROAD_Y_DIR,     ///< Road placement (Y axis)
	DDSP_PLACE_AUTOROAD,       ///< Road placement (auto)
	DDSP_BUILD_BUSSTOP,        ///< Road stop placement (buses)
	DDSP_BUILD_TRUCKSTOP,      ///< Road stop placement (trucks)
	DDSP_REMOVE_BUSSTOP,       ///< Road stop removal (buses)
	DDSP_REMOVE_TRUCKSTOP,     ///< Road stop removal (trucks)
	DDSP_CONVERT_ROAD,         ///< Road conversion
};


/**
 * Target of the viewport scrolling GS method
 */
enum ViewportScrollTarget {
	VST_EVERYONE, ///< All players
	VST_COMPANY,  ///< All players in specific company
	VST_CLIENT,   ///< Single player
};

/** Enumeration of multi-part foundations */
enum FoundationPart {
	FOUNDATION_PART_NONE     = 0xFF,  ///< Neither foundation nor groundsprite drawn yet.
	FOUNDATION_PART_NORMAL   = 0,     ///< First part (normal foundation or no foundation)
	FOUNDATION_PART_HALFTILE = 1,     ///< Second part (halftile foundation)
	FOUNDATION_PART_END
};

#endif /* VIEWPORT_TYPE_H */
