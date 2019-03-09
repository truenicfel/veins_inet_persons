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

#include "veins/modules/mobility/traci/VehicleSubscriptionManager.h"
#include "veins/modules/mobility/traci/TraCIConstants.h"

VehicleSubscriptionManager::VehicleSubscriptionManager()
    : mSubscribedVehicleIds()
    , mUpdatedVehicles()
    , mDisappearedVehicles()
    , mConnection(nullptr)
    , mCommandInterface(nullptr)
{
}

void VehicleSubscriptionManager::update(std::list<std::string>& currentlyActiveVehicleIds) {
    processVehicleIDList(currentlyActiveVehicleIds);
}

bool VehicleSubscriptionManager::update(TraCIBuffer& buffer) {
    bool idListUpdateReceived = false;

    // this is the object id that this subscription result contains
    // content about. for example a vehicle id.
    std::string responseObjectID;
    buf >> responseObjectID;

    // the number of response variables that are contained in this buffer
    uint8_t numberOfResponseVariables;
    buf >> numberOfResponseVariables;
    // this should either be one or nine
    ASSERT(numberOfResponseVariables == 1 || numberOfResponseVariables == 9);

    // these need to be filled
    double x;
    double y;
    double speed;
    std::string edge;
    double angle;
    int signals;
    double length;
    double height;
    double width;
    std::string typeID;


    for (int counter = 0; counter < numberOfResponseVariables; ++counter) {

        // extract a couple of values:
        // - identifies the variable (position, list etc.)
        uint8_t responseVariableID;
        buf >> responseVariableID;
        // - status of the variable
        uint8_t variableStatus;
        buf >> variableStatus;
        // - type of the variable
        uint8_t variableType;
        buf >> variableType;

        if (variableStatus == RTYPE_OK) {
            // the status of the variable is ok

            // now check which variable id we got
            // it is either an id_list or a subscription for a specific person
            // never both
            if (responseVariableID == ID_LIST) {
                ASSERT(varType == TYPE_STRINGLIST);
                idListUpdateReceived = true;

                uint32_t numberOfActiveVehicles;
                buffer >> numberOfActiveVehicles;
                EV_DEBUG << "TraCI reports " << numberOfActiveVehicles << " active vehicles." << endl;

                // add all id strings of reported active persons to a set
                std::set<std::string> traciActiveVehicles;
                for (uint32_t counter = 0; counter < numberOfActiveVehicles; ++counter) {
                    std::string idstring;
                    buffer >> idstring;
                    traciActiveVehicles.insert(idstring);
                }

                // helper method takes care of this
                processVehicleIDList(traciActiveVehicles);

            } else {
                // subscription for specific vehicle
                if (responseVariableID == VAR_POSITION) {
                    ASSERT(varType == POSITION_2D);
                    buffer >> x;
                    buffer >> y;
                } else if (responseVariableID == VAR_ROAD_ID) {
                    ASSERT(varType == TYPE_STRING);
                    buffer >> edge;
                } else if (responseVariableID == VAR_SPEED) {
                    ASSERT(varType == TYPE_DOUBLE);
                    buffer >> speed;
                    numberOfReadVariables++;
                } else if (responseVariableID == VAR_ANGLE) {
                    ASSERT(varType == TYPE_DOUBLE);
                    buffer >> angle;
                } else if (variable1_resp == VAR_SIGNALS) {
                    ASSERT(varType == TYPE_INTEGER);
                    buffer >> signals;
                }
                else if (variable1_resp == VAR_LENGTH) {
                    ASSERT(varType == TYPE_DOUBLE);
                    buffer >> length;
                }
                else if (variable1_resp == VAR_HEIGHT) {
                    ASSERT(varType == TYPE_DOUBLE);
                    buffer >> height;
                }
                else if (variable1_resp == VAR_WIDTH) {
                    ASSERT(varType == TYPE_DOUBLE);
                    buffer >> width;
                }
                else if (variable1_resp == VAR_TYPE) {
                    ASSERT(varType == TYPE_STRING);
                    buffer >> typeID;
                }
                else {
                    error("Received unknown vehicle subscription result");
                }
            }

        } else {
            // the status of the variable is not ok
            ASSERT(varType == TYPE_STRING);
            std::string errormsg;
            buf >> errormsg;
            if (isSubscribed(responseObjectID)) {
                if (variableStatus == RTYPE_NOTIMPLEMENTED) {
                    error(
                            "TraCI server reported subscribing to vehicle variable 0x%2x not implemented (\"%s\"). Might need newer version.",
                            responseVariableID, errormsg.c_str());
                }

                error(
                        "TraCI server reported error subscribing to vehicle variable 0x%2x (\"%s\").",
                        responseVariableID, errormsg.c_str());
            }
        }

        // make sure we are only entering this section if we got a vehicle that we already subscribed to
        if (isSubscribed(responseObjectID)) {
            // we want to deliver an update for this vehicle for the next call to getUpdated()
            TraCIVehicle vehicle(x, y, edge, speed, angle, responseObjectID, typeID, signals, length, height, width);
            mUpdatedVehicles.push_back(vehicle);
        }

    }

    return idListUpdateReceived;
}

std::list<TraCIVehicle> VehicleSubscriptionManager::getUpdated() {
    std::list<TraCIVehicle> temp = mUpdatedPersons;
    mUpdatedVehicles.clear();
    return temp;
}

std::set<std::string> VehicleSubscriptionManager::getDisappeared() {
    std::list<TraCIVehicle> temp = mDisappearedVehicles;
    mDisappearedVehicles.clear();
    return temp;
}

void VehicleSubscriptionManager::initialize(std::unique_ptr<TraCIConnection> connection, std::unique_ptr<TraCICommandInterface> commandInterface) {
    mConnection = connection;
    mCommandInterface = commandInterface;

    // initialize subscriptions
    // subscribe to list of vehicle ids
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = "";
    uint8_t variableNumber = 1;
    uint8_t variable1 = ID_LIST;
    TraCIBuffer buf = mConnection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1);

    // remove unnecessary stuff from buffer
    uint8_t responseCommandLength;
    buf >> responseCommandLength;
    ASSERT(responseCommandLength == 0);
    // this is the length of the command
    uint32_t responseCommandLengthExtended;
    buf >> responseCommandLengthExtended;
    uint8_t responseCommandID;
    buf >> responseCommandID;
    ASSERT(responseCommandID == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE);

    update(buf);

    ASSERT(buf.eof());

}

void VehicleSubscriptionManager::processVehicleIDList(std::list<std::string>& idList) {

    // check for vehicles that need subscribing to
    std::set<std::string> needSubscribe;
    // basically: idList - mSubscribedVehicleIds
    // --> result is a list of vehicles that need to be subscribed
    std::set_difference(idList.begin(), idList.end(),
            mSubscribedVehicleIds.begin(), mSubscribedVehicleIds.end(),
            std::inserter(needSubscribe, needSubscribe.begin()));
    for (auto id : needSubscribe) {
        mSubscribedVehicleIds.insert(id);
        // the vehicle will be automatically added to update
        // after executing the following method
        subscribeToVehicleVariables(id);
    }

    // check for vehicles that disappeared
    // basically: mSubscribedVehicleIds - idList
    // --> result is a list of vehicles that need to be unsubscribed
    std::set_difference(mSubscribedVehicleIds.begin(), mSubscribedVehicleIds.end(),
            idList.begin(), idList.end(),
            std::inserter(mDisappearedVehicles, mDisappearedVehicles.begin()));
    for (auto id : mDisappearedVehicles) {
        if (isSubscribed(id)) {
            mSubscribedVehicleIds.erase(id);
        }
        // there is no need to unsubscribe at TraCI: if a vehicle disappears
        // the subscription will not be updated anyways
    }
}

void VehicleSubscriptionManager::subscribeToVehicleVariables(std::string id) {
    // subscribe to some attributes of the vehicle
    simtime_t beginTime = 0;
    simtime_t endTime = SimTime::getMaxTime();
    std::string objectId = vehicleId;
    uint8_t variableNumber = 9;
    uint8_t variable1 = VAR_POSITION;
    uint8_t variable2 = VAR_ROAD_ID;
    uint8_t variable3 = VAR_SPEED;
    uint8_t variable4 = VAR_ANGLE;
    uint8_t variable5 = VAR_SIGNALS;
    uint8_t variable6 = VAR_LENGTH;
    uint8_t variable7 = VAR_HEIGHT;
    uint8_t variable8 = VAR_WIDTH;
    uint8_t variable9 = VAR_TYPE;

    TraCIBuffer buffer = mConnection->query(CMD_SUBSCRIBE_VEHICLE_VARIABLE, TraCIBuffer() << beginTime << endTime << objectId << variableNumber << variable1 << variable2 << variable3 << variable4 << variable5 << variable6 << variable7 << variable8 << variable9);

    // remove unnecessary stuff from buffer
    uint8_t responseCommandLength;
    buf >> responseCommandLength;
    ASSERT(responseCommandLength == 0);
    // this is the length of the command
    uint32_t responseCommandLengthExtended;
    buf >> responseCommandLengthExtended;
    uint8_t responseCommandID;
    buf >> responseCommandID;
    ASSERT(responseCommandID == RESPONSE_SUBSCRIBE_VEHICLE_VARIABLE);

    update(buffer);
    ASSERT(buffer.eof());
}

bool VehicleSubscriptionManager::isSubscribed(std::string id) {
    return mSubscribedVehicleIds.find(id) != mSubscribedVehicleIds.end();
}


