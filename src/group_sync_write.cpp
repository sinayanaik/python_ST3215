#include "st3215/group_sync_write.h"

namespace st3215 {

GroupSyncWrite::GroupSyncWrite(ProtocolPacketHandler* ph, uint8_t start_address, uint8_t data_length)
    : ph_(ph), start_address_(start_address), data_length_(data_length), is_param_changed_(false) {
    clearParam();
}

void GroupSyncWrite::makeParam() {
    if (data_dict_.empty()) {
        return;
    }

    param_.clear();

    for (const auto& [sts_id, data] : data_dict_) {
        if (data.empty()) {
            return;
        }
        param_.push_back(sts_id);
        param_.insert(param_.end(), data.begin(), data.end());
    }
}

bool GroupSyncWrite::addParam(uint8_t sts_id, const std::vector<uint8_t>& data) {
    if (data_dict_.find(sts_id) != data_dict_.end()) {
        return false;
    }

    if (data.size() > data_length_) {
        return false;
    }

    data_dict_[sts_id] = data;
    is_param_changed_ = true;
    return true;
}

void GroupSyncWrite::removeParam(uint8_t sts_id) {
    if (data_dict_.find(sts_id) == data_dict_.end()) {
        return;
    }

    data_dict_.erase(sts_id);
    is_param_changed_ = true;
}

bool GroupSyncWrite::changeParam(uint8_t sts_id, const std::vector<uint8_t>& data) {
    if (data_dict_.find(sts_id) == data_dict_.end()) {
        return false;
    }

    if (data.size() > data_length_) {
        return false;
    }

    data_dict_[sts_id] = data;
    is_param_changed_ = true;
    return true;
}

void GroupSyncWrite::clearParam() {
    data_dict_.clear();
}

int GroupSyncWrite::txPacket() {
    if (data_dict_.empty()) {
        return COMM_NOT_AVAILABLE;
    }

    if (is_param_changed_ || param_.empty()) {
        makeParam();
    }

    return ph_->syncWriteTxOnly(start_address_, data_length_, param_,
                                 data_dict_.size() * (1 + data_length_));
}

}  // namespace st3215
