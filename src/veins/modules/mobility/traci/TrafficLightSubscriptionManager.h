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

#ifndef SRC_VEINS_MODULES_MOBILITY_TRACI_TRAFFICLIGHTSUBSCRIPTIONMANAGER_H_
#define SRC_VEINS_MODULES_MOBILITY_TRACI_TRAFFICLIGHTSUBSCRIPTIONMANAGER_H_

#include <string>

#include "veins/modules/mobility/traci/SubscriptionManagerBase.h"
#include "veins/modules/mobility/traci/TraCITrafficLight.h"

namespace Veins {

class TrafficLightSubscriptionManager: public SubscriptionManagerBase {

public:

    /**
     * Constructor.
     */
    TrafficLightSubscriptionManager();

    /**
     * Default destructor.
     */
    virtual ~TrafficLightSubscriptionManager() = default;

    /**
     * Update this manager with the given buffer. The next call
     * to getTrafficLightUpdates() will change after a call to this.
     *
     * @param buffer the buffer containing the subscription information.
     *
     * @return true (default)
     */
    bool update(TraCIBuffer& buffer);

    /**
     * Subscribe to a specific traffic light.
     *
     * @param id of the traffic light to subscribe to.
     */
    void subscribeToTrafficLight(std::string id);

    /**
     * Get the updates to all traffic lights since the last time
     * you called this method.
     */
    std::list<TraCITrafficLight> getUpdated();

private:

    /**
     * Stores the updates for traffic lights since the last time
     * getTrafficLightUpdates() got called.
     */
    std::list<TraCITrafficLight> mUpdatedTrafficLights;
};

} /* namespace Veins */

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_TRAFFICLIGHTSUBSCRIPTIONMANAGER_H_ */
