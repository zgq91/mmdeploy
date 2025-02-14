// Copyright (c) OpenMMLab. All rights reserved.
#ifndef MMDEPLOY_TEST_RESOURCE_H
#define MMDEPLOY_TEST_RESOURCE_H
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "test_define.h"

#if __GNUC__ >= 8
#include <filesystem>
namespace fs = std::filesystem;
#else

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
#endif

using namespace std;

class MMDeployTestResources {
 public:
  static MMDeployTestResources &Get() {
    static MMDeployTestResources resource;
    return resource;
  }

  const std::vector<std::string> &device_names() const { return devices_; }
  const std::vector<std::string> &device_names(const std::string &backend) const {
    return backend_devices_.at(backend);
  }
  const std::vector<std::string> &backends() const { return backends_; }
  const std::vector<std::string> &codebases() const { return codebases_; }
  const std::string &resource_root_path() const { return resource_root_path_; }

  bool HasDevice(const std::string &name) const {
    return std::any_of(devices_.begin(), devices_.end(),
                       [&](const std::string &device_name) { return device_name == name; });
  }

  bool IsDir(const std::string &dir_name) const {
    fs::path path{resource_root_path_ + "/" + dir_name};
    return fs::is_directory(path);
  }

  bool IsFile(const std::string &file_name) const {
    fs::path path{resource_root_path_ + "/" + file_name};
    return fs::is_regular_file(path);
  }

 public:
  std::vector<std::string> LocateModelResources(const std::string &sdk_model_zoo_dir) {
    std::vector<std::string> sdk_model_list;
    if (resource_root_path_.empty()) {
      return sdk_model_list;
    }

    fs::path path{resource_root_path_ + "/" + sdk_model_zoo_dir};
    if (!fs::is_directory(path)) {
      return sdk_model_list;
    }
    for (auto const &dir_entry : fs::directory_iterator{path}) {
      fs::directory_entry entry{dir_entry.path()};
      if (auto const &_path = dir_entry.path(); fs::is_directory(_path)) {
        sdk_model_list.push_back(dir_entry.path());
      }
    }
    return sdk_model_list;
  }

  std::vector<std::string> LocateImageResources(const std::string &img_dir) {
    std::vector<std::string> img_list;

    if (resource_root_path_.empty()) {
      return img_list;
    }

    fs::path path{resource_root_path_ + "/" + img_dir};
    if (!fs::is_directory(path)) {
      return img_list;
    }

    set<string> extensions{".png", ".jpg", ".jpeg", ".bmp"};
    for (auto const &dir_entry : fs::directory_iterator{path}) {
      if (!fs::is_regular_file(dir_entry.path())) {
        std::cout << dir_entry.path().string() << std::endl;
        continue;
      }
      auto const &_path = dir_entry.path();
      auto ext = _path.extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
      if (extensions.find(ext) != extensions.end()) {
        img_list.push_back(_path.string());
      }
    }
    return img_list;
  }

 private:
  MMDeployTestResources() {
    devices_ = Split(kDevices);
    backends_ = Split(kBackends);
    codebases_ = Split(kCodebases);
    backend_devices_["pplnn"] = {"cpu", "cuda"};
    backend_devices_["trt"] = {"cuda"};
    backend_devices_["ort"] = {"cpu"};
    backend_devices_["ncnn"] = {"cpu"};
    backend_devices_["openvino"] = {"cpu"};
    resource_root_path_ = LocateResourceRootPath(fs::current_path(), 8);
  }

  static std::vector<std::string> Split(const std::string &text, char delimiter = ';') {
    std::vector<std::string> result;
    std::istringstream ss(text);
    for (std::string word; std::getline(ss, word, delimiter);) {
      result.emplace_back(word);
    }
    return result;
  }

  std::string LocateResourceRootPath(const fs::path &cur_path, int max_depth) {
    if (max_depth < 0) {
      return "";
    }
    for (auto const &dir_entry : fs::directory_iterator{cur_path}) {
      fs::directory_entry entry{dir_entry.path()};
      auto const &_path = dir_entry.path();
      if (fs::is_directory(_path) && _path.filename() == "mmdeploy_test_resources") {
        return _path.string();
      }
    }
    // Didn't find 'mmdeploy_test_resources' in current directory.
    // Move to its parent directory and keep looking for it
    return LocateResourceRootPath(cur_path.parent_path(), max_depth - 1);
  }

 private:
  std::vector<std::string> devices_;
  std::vector<std::string> backends_;
  std::vector<std::string> codebases_;
  std::map<std::string, std::vector<std::string>> backend_devices_;
  std::string resource_root_path_;
};

#endif  // MMDEPLOY_TEST_RESOURCE_H
