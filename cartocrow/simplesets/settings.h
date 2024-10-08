#ifndef CARTOCROW_SETTINGS_H
#define CARTOCROW_SETTINGS_H

#include "types.h"

namespace cartocrow::simplesets {
struct GeneralSettings {
	/// Radius of circle that represents a point.
	Number<Inexact> pointSize;
	/// Maximum number of inflections a bank is allowed to have.
	int inflectionLimit;
	/// Maximum total angle of a bend (maximum monotone subsequence of a bank).
	Number<Inexact> maxBendAngle;
	/// Maximum turning angle in a bank.
	Number<Inexact> maxTurnAngle;

	/// The distance each pattern is dilated.
	Number<Inexact> dilationRadius() const {
		return pointSize * 3;
	}
};

struct PartitionSettings {
	/// Create banks?
	bool banks;
	/// Create islands?
	bool islands;
	/// Delay merges that create patterns whose points are not distributed 'regularly'?
	/// A pattern is not regular if it has clearly discernible sub-patterns.
	bool regularityDelay;
	/// Delay merges that create patterns that intersect points.
	bool intersectionDelay;
	/// Disallow merges that have a point within distance admissibleRadiusFactor * dilationRadius.
	Number<Inexact> admissibleRadiusFactor;
};

struct ComputeDrawingSettings {
	/// Aim to keep a disk around each point visible of radius cutoutRadiusFactor * dilationRadius.
	Number<Inexact> cutoutRadiusFactor;
};

struct DrawSettings {
	std::vector<Color> colors;
	Number<Inexact> whiten;
	Number<Inexact> pointStrokeWeight(GeneralSettings gs) const {
		return gs.pointSize / 2.5;
	}
	Number<Inexact> contourStrokeWeight(GeneralSettings gs) const {
		return gs.pointSize / 3.5;
	}
	Color getColor(int category) const {
		Color fillColor;
		if (category > colors.size() || category < 0) {
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
