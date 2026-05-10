#include "CameraController.h"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/window.hpp>

using namespace godot;

void CameraController::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_player1_path"), &CameraController::get_player1_path);
    ClassDB::bind_method(D_METHOD("set_player1_path", "path"), &CameraController::set_player1_path);
    ClassDB::bind_method(D_METHOD("get_player2_path"), &CameraController::get_player2_path);
    ClassDB::bind_method(D_METHOD("set_player2_path", "path"), &CameraController::set_player2_path);
    ClassDB::bind_method(D_METHOD("update_player_references"), &CameraController::update_player_references);
    ClassDB::bind_method(D_METHOD("update_camera"), &CameraController::update_camera);
    
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "player1_path"), "set_player1_path", "get_player1_path");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "player2_path"), "set_player2_path", "get_player2_path");
}

CameraController::CameraController() {
    // 构造函数
}

CameraController::~CameraController() {
    // 析构函数
}

void CameraController::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    
    UtilityFunctions::print("CameraController: _ready() called");
    
    // 设置相机为当前相机
    set_enabled(true);
    make_current();
    
    // 获取玩家引用
    update_player_references();
    
    if (player1 && player2) {
        UtilityFunctions::print("CameraController: Both players found");
    } else {
        UtilityFunctions::printerr("CameraController: Failed to find players");
    }
}

void CameraController::_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }
    
    // 每帧更新摄像机
    update_camera();
}

void CameraController::update_player_references() {
    // 方法1：尝试从当前节点的父节点查找（如果玩家和相机是兄弟节点）
    Node* parent = get_parent();
    if (parent) {
        // 查找玩家1
        Node* player1_node = parent->get_node_or_null(NodePath(player1_path));
        if (player1_node) {
            player1 = Object::cast_to<Node2D>(player1_node);
            if (player1) {
                UtilityFunctions::print(vformat("CameraController: Found player1 (sibling): %s", player1->get_name()));
                // 如果找到了玩家1，继续找玩家2
                Node* player2_node = parent->get_node_or_null(NodePath(player2_path));
                if (player2_node) {
                    player2 = Object::cast_to<Node2D>(player2_node);
                    if (player2) {
                        UtilityFunctions::print(vformat("CameraController: Found player2 (sibling): %s", player2->get_name()));
                        return; // 两个都找到了，直接返回
                    }
                }
            }
        }
    }
    
    // 方法2：如果兄弟节点中没找到，尝试从场景根节点查找
    // 获取当前场景
    Node* current_scene = get_tree()->get_current_scene();
    if (current_scene) {
        UtilityFunctions::print(vformat("CameraController: Current scene: %s", current_scene->get_name()));
        
        // 查找玩家1
        if (!player1) {
            Node* player1_node = current_scene->get_node_or_null(NodePath(player1_path));
            if (player1_node) {
                player1 = Object::cast_to<Node2D>(player1_node);
                if (player1) {
                    UtilityFunctions::print(vformat("CameraController: Found player1 (scene): %s", player1->get_name()));
                }
            }
        }
        
        // 查找玩家2
        if (!player2) {
            Node* player2_node = current_scene->get_node_or_null(NodePath(player2_path));
            if (player2_node) {
                player2 = Object::cast_to<Node2D>(player2_node);
                if (player2) {
                    UtilityFunctions::print(vformat("CameraController: Found player2 (scene): %s", player2->get_name()));
                }
            }
        }
    } else {
        UtilityFunctions::printerr("CameraController: No current scene found");
    }
    
    // 如果还是没找到，输出错误信息
    if (!player1) {
        UtilityFunctions::printerr(vformat("CameraController: Could not find player1 with path: %s", player1_path));
    }
    if (!player2) {
        UtilityFunctions::printerr(vformat("CameraController: Could not find player2 with path: %s", player2_path));
    }
}

void CameraController::update_camera() {
    if (!player1 || !player2) {
        static int retry_count = 0;
        if (retry_count < 5) {
            // 前5帧尝试重新获取玩家引用
            update_player_references();
            retry_count++;
        }
        if (!player1 || !player2) {
            return;
        }
    }
    
    // 计算两个玩家的中心点
    Vector2 pos1 = player1->get_global_position();
    Vector2 pos2 = player2->get_global_position();
    Vector2 center = (pos1 + pos2) * 0.5f;
    
    // 设置摄像机位置
    set_global_position(center);
    
    // 调试输出（每60帧输出一次）
    static int frame_count = 0;
    frame_count++;
    if (frame_count % 60 == 0) {
        UtilityFunctions::print(vformat("CameraController: Player1=(%.1f, %.1f), Player2=(%.1f, %.1f), Center=(%.1f, %.1f)", 
            pos1.x, pos1.y, pos2.x, pos2.y, center.x, center.y));
    }
}

// Getter 和 Setter
String CameraController::get_player1_path() const {
    return player1_path;
}

void CameraController::set_player1_path(const String& p_path) {
    player1_path = p_path;
    // 路径改变时重新获取玩家引用
    update_player_references();
}

String CameraController::get_player2_path() const {
    return player2_path;
}

void CameraController::set_player2_path(const String& p_path) {
    player2_path = p_path;
    // 路径改变时重新获取玩家引用
    update_player_references();
}