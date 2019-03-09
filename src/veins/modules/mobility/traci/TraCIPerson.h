//
// Copyright (C) 2006-2017 Christoph Sommer <sommer@ccs-labs.org>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_TRACIPERSON_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_TRACIPERSON_H_

#include "veins/modules/mobility/traci/TrafficParticipant.h"

/**
 * @class TraCIPerson
 *
 * TraCIPerson is nothing more than a simple TrafficParticipant
 * but can be extended in the future.
 */
class TraCIPerson: public TrafficParticipant {

    /**
     * Constructor.
     *
     * @param x coordinate to initialize this participant with.
     * @param y coordinate to initialize this participant with.
     * @param edgeID to initialize this participant with.
     * @param speed to initialize this participant with.
     * @param angle to initialize this participant with.
     * @param id to initialize this participant with.
     * @param typeID to initialize this participant with.
     */
    TraCIPerson(double x, double y, std::string edgeID, double speed, double angle, std::string id, std::string typeID);

    /**
     * Default destructor.
     */
    ~TraCIPerson() = default;

};

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_TRACIPERSON_H_ */
