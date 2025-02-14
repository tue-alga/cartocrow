#ifndef CARTOCROW_SETTINGS_H
#define CARTOCROW_SETTINGS_H

#include "types.h"

namespace cartocrow::simplesets {
struct GeneralSettings {
	/// Radius of circle that represents a point.
	Number<Inexact> pointSize = 1.0;
	/// Maximum number of inflections a bank is allowed to have.
	/// The current implementation assumes this is at most 2.
	int inflectionLimit = 2;
	/// Maximum total angle (in radians) of a bend (maximum monotone subsequence of a bank).
	/// The current implementation assumes this is at most pi.
	Number<Inexact> maxBendAngle = M_PI;
	/// Maximum turning angle (in radians) in a bank.
	/// The current implementation assumes this is less than pi.
	Number<Inexact> maxTurnAngle = 70.0 / 180 * M_PI;

	/// The distance each pattern is dilated.
	[[nodiscard]] Number<Exact> dilationRadius() const {
		return pointSize * 3;
	}
};

struct PartitionSettings {
	/// Create banks?
	bool banks = true;
	/// Create islands?
	bool islands = true;
	/// Delay merges that create patterns whose points are not distributed 'regularly'?
	/// A pattern is not regular if it has clearly discernible sub-patterns.
	bool regularityDelay = true;
	/// Delay merges that create patterns that intersect points.
	/// This generally has little effect on the partitions but does significantly increase the running time.
	bool intersectionDelay = true;
	/// Disallow merges that have a point within distance admissibleRadiusFactor * dilationRadius.
	Number<Inexact> admissibleRadiusFactor = 0.5;
};

struct ComputeDrawingSettings {
	/// Aim to keep a disk around each point visible of radius cutoutRadiusFactor * dilationRadius.
	Number<Inexact> cutoutRadiusFactor = 0.675;
	/// Apply smoothing to cutouts
	bool smooth = true;
	/// The amount cutouts are smoothed (if applied).
	/// More precisely, this is the radius of erosion and dilation applied as a factor of the dilation radius.
	/// The value should not be set higher than 0.2.
	Number<Inexact> smoothingRadiusFactor = 0.2;
};

struct DrawSettings {
	/// Category i will be drawn with colors[i].
	/// Shape fills are first mixed with white.
	/// \sa whiten
	std::vector<Color> colors;
	/// The proportion of white mixed into the color when filling patterns.
	Number<Inexact> whiten = 0.7;
	Number<Inexact> pointStrokeWeight(GeneralSettings gs) const {
		return gs.pointSize / 2.5;
	}
	Number<Inexact> contourStrokeWeight(GeneralSettings gs) const {
		return gs.pointSize / 3.5;
	}
	Color getColor(int category) const {
		Color fillColor;
		if (category >= colors.size() || category < 0) {
			std::cerr << "Warning! No color specified for category " << category << std::endl;
			fillColor = Color{240, 240, 240};
		} else {
			fillColor = colors[category];
		}
		return fillColor;
	}
};

struct Settings {
	GeneralSettings gs;
	PartitionSettings ps;
	ComputeDrawingSettings cds;
	DrawSettings ds;
};
}


#endif //CARTOCROW_SETTINGS_H
