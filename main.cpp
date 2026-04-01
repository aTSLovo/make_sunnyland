#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>

int main(int, char**) {
    // 设置最低打印消息等级，默认为info
    spdlog::set_level(spdlog::level::info);

    // 消息
    spdlog::trace("最低级的log!");
    spdlog::debug("调试消息!");
    spdlog::info("消息: hello world!");
    spdlog::warn("警告信息!");
    spdlog::error("出错!");
    spdlog::critical("高级别的log!");

    spdlog::info("日志输出：{}, {}, {}", "2026.04.01", "hello", 11.41);

    try {
        // 1. 加载json文件
        std::ifstream imput_file("assets/json_example.json");
        nlohmann::ordered_json json_data = nlohmann::ordered_json::parse(imput_file);
        imput_file.close();
        spdlog::info("成功加载json文件");

        // 2. 获取不同类型的数据
        // 2.1 字符串类型
        std::string name = json_data.at("name").get<std::string>();
        spdlog::info("Name: {}", name);

        // 2.2 数字
        int age = json_data["age"].get<int>();
        double height  = json_data["height_meters"].get<double>();
        spdlog::info("Age: {}", age);
        spdlog::info("Height: {}", height);

        // 2.3 检查是否为null
        // 中括号里的键不存在，会告知是null
        // std::string email = json_data["email_"].get<std::string>();
        // spdlog::info("Email: {}", email);
        // 但是at会检测out_of_range，结论中括号可以获取一个不存在的键，但是at方法不行
        std::string email = json_data.at("email").get<std::string>();
        spdlog::info("Email: {}", email);
        
        // 3 安全访问的方法
        // 3.1 .contains()检查某个键是否存在，如不存在则返回false
        if(json_data.contains("email")) {
            spdlog::info("Email: {}", json_data["email"].get<std::string>());
        }
        if(json_data.contains("nonExistentKey")) {
            spdlog::info("nonExistentKey is found");
        }
        else {
            spdlog::info("nonExistentKey is not found");
        }

        // 3.2 .value()获取一个可能存在的键，如不存在则返回指定默认值(第二个参数)
        std::string optional_value = json_data.value("optionalKey", "default_string_value");
        int optional_int = json_data.value("optionalNumber", 42);
        spdlog::info("Optional Key(string): {}", optional_value);
        spdlog::info("Optional Key(int): {}", optional_int);

        // 4 对象(大括号嵌套)
        nlohmann::ordered_json address_obj = json_data["address"];
        std::string street = address_obj["street"].get<std::string>();
        std::string city = address_obj.value("city", "Unknown City"); // 使用 .value() 提供默认值
        bool isPrimaryAddr = address_obj.value("isPrimary", false);   // 访问对象内的布尔值
        spdlog::info("Address: {}, {}", street, city);
        spdlog::info("Is Primary Address: {}", isPrimaryAddr);

        // 5 数组
        // 5.1 数组 (Array) - 字符串数组
        spdlog::info("Hobbies: ");
        nlohmann::ordered_json hobbies_array = json_data["hobbies"];
        for(const auto& hobby : hobbies_array) {
            spdlog::info(" - {}", hobby.get<std::string>());
        }

        // 5.2 数组 (Array) - 数字数组
        spdlog::info("Scores:");
        for(const auto& score : json_data["scores"]) {
            if(score.is_number_integer()) {
                spdlog::info(" - {}", score.get<int>());
            }
            else if(score.is_number_float()) {
                spdlog::info(" - {}", score.get<double>());
            }
        }

        // 5.3 数组 (Array) - 对象数组
        spdlog::info("projects:");
        for(const auto& project : json_data["projects"]) {
            std::string projectName = project["projectName"].get<std::string>();
            std::string status = project["status"].get<std::string>();
            double budget = project.value("budget", 0.0); // 使用 value 获取，若不存在则为0.0
            bool isActive = project.value("isActive", false);

            spdlog::info("  ProjectName: {}", projectName);
            spdlog::info("  Status: {}", status);
            spdlog::info("  Budget: {}", budget);
            spdlog::info("  Is Active: {}", isActive);

            if(project.contains("deadline")) {
                if(project["deadline"].is_null()) {
                    spdlog::info("  deadline: null");
                }
                else {
                    spdlog::info("  deadline: {}", project["deadline"].get<std::string>());
                }
            }
            spdlog::info("--------------------------------");
        }

        // 5.4 直接访问更深层嵌套的对象和数组
        double metadata_version = json_data["metadata"]["version"].get<double>();
        spdlog::info("Metadata Version: {}", metadata_version);
        spdlog::info("Metadata Tags:");
        for(const auto& tag_array : json_data.at("metadata").at("tags")) {
            std::string tag = tag_array.get<std::string>();
            spdlog::info("  - {}", tag);
        }

        // 6 保存json数据到文件
        std::ofstream output_file("assets/json_save.json");
        output_file << json_data.dump(4); // 使用 dump(4) 进行格式化输出，缩进为4个空格
        output_file.close();
        spdlog::info("JSON 数据已保存到文件 assets/json_save.json");
    } catch (const std::exception& e) {
        spdlog::error("Exception: {}", e.what());
        // return EXIT_FAILURE;
    }
    return 0;
}