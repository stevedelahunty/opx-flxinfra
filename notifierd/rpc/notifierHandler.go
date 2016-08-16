//
//Copyright [2016] [SnapRoute Inc]
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//       Unless required by applicable law or agreed to in writing, software
//       distributed under the License is distributed on an "AS IS" BASIS,
//       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//       See the License for the specific language governing permissions and
//       limitations under the License.
//
// _______  __       __________   ___      _______.____    __    ____  __  .___________.  ______  __    __
// |   ____||  |     |   ____\  \ /  /     /       |\   \  /  \  /   / |  | |           | /      ||  |  |  |
// |  |__   |  |     |  |__   \  V  /     |   (----` \   \/    \/   /  |  | `---|  |----`|  ,----'|  |__|  |
// |   __|  |  |     |   __|   >   <       \   \      \            /   |  |     |  |     |  |     |   __   |
// |  |     |  `----.|  |____ /  .  \  .----)   |      \    /\    /    |  |     |  |     |  `----.|  |  |  |
// |__|     |_______||_______/__/ \__\ |_______/        \__/  \__/     |__|     |__|      \______||__|  |__|
//

package rpc

import (
	"errors"
	"infra/notifierd/api"
	"notifierd"
)

func (h *rpcServiceHandler) CreateNotifierEnable(conf *notifierd.NotifierEnable) (bool, error) {
	h.logger.Info("CreateNotifierEnable is not supported", conf)
	return false, errors.New("CreateNotifierEnable is not supported")
}

func (h *rpcServiceHandler) DeleteNotifierEnable(conf *notifierd.NotifierEnable) (bool, error) {
	h.logger.Info("DeleteNotifierEnable is not supported", conf)
	return false, errors.New("DeleteNotifierEnable is not supported")
}

func (h *rpcServiceHandler) UpdateNotifierEnable(oldCfg *notifierd.NotifierEnable, newCfg *notifierd.NotifierEnable, attrset []bool, op []*notifierd.PatchOpInfo) (bool, error) {
	h.logger.Info("UpdateNotifierEnable is not supported", oldCfg, newCfg, attrset, op)
	convOldCfg := convertFromRPCFmtNotifierEnable(oldCfg)
	convNewCfg := convertFromRPCFmtNotifierEnable(newCfg)
	return api.UpdateNotifierEnable(convOldCfg, convNewCfg, attrset)
}
