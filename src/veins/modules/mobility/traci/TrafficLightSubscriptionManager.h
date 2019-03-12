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

#include "veins/modules/mobility/traci/TraCIConnection.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"
#include "veins/modules/mobility/traci/TraCITrafficLight.h"
#include "veins/modules/mobility/traci/TraCIBuffer.h"

namespace Veins {

class TrafficLightSubscriptionManager {
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
     * Initialize this manager with the given parameters to access TraCI.
     *
     * @param connection to access traci
     * @param commandInterface to access traci
     */
    void initialize(std::shared_ptr<TraCIConnection> connection, std::shared_ptr<TraCICommandInterface> commandInterface);

    /**
     * Update this manager with the given buffer. The next call
     * to getTrafficLightUpdates() will change after a call to this.
     *
     * @param buffer the buffer containing the subscription information.
     */
    void update(TraCIBuffer& buffer);

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

    /**
     * A set of subscribed traffic lights identified by their id.
     */
    std::set<std::string> mSubscribedTrafficLights;

    /**
     * Stores connection to access TraCI.
     */
    std::shared_ptr<TraCIConnection> mConnection;

    /**
     * Stores command interface to access TraCI.
     */
    std::shared_ptr<TraCICommandInterface> mCommandInterface;

    /**
     * Check if the given id is subscribed.
     */
    bool isSubscribed(std::string id);
};

} /* namespace Veins */

#endif /* SRC_VEINS_MODULES_MOBILITY_TRACI_TRAFFICLIGHTSUBSCRIPTIONMANAGER_H_ */
