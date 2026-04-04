#include "player.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

// 构造函数：初始化默认值
Player::Player() : move_speed(300.0f), jump_force(400.0f) {
    // 不需要初始化is_on_floor，因为使用父类方法
}

Player::~Player() {
    // 清理资源（如有）
}

// 绑定Godot方法、属性和信号
void Player::_bind_methods() {
    // 注册属性
    ClassDB::bind_method(D_METHOD("get_move_speed"), &Player::get_move_speed);
    ClassDB::bind_method(D_METHOD("set_move_speed", "speed"), &Player::set_move_speed);
    ClassDB::bind_method(D_METHOD("get_jump_force"), &Player::get_jump_force);
    ClassDB::bind_method(D_METHOD("set_jump_force", "force"), &Player::set_jump_force);

    // 注册方法
    ClassDB::bind_method(D_METHOD("move", "direction"), &Player::move);
    ClassDB::bind_method(D_METHOD("jump"), &Player::jump);
    // 移除了handle_collision方法的注册

    // 添加属性到Godot编辑器（可选）
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_speed"), "set_move_speed", "get_move_speed");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_force"), "set_jump_force", "get_jump_force");
}

// 节点就绪时调用
void Player::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return; // 在编辑器中跳过
    }
    // 初始化代码，如获取子节点
    // 可以在这里设置初始速度
    set_velocity(Vector2(0, 0));
}

// 物理处理每帧调用
void Player::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    // 获取输入
    Input *input = Input::get_singleton();
    float direction = 0.0f;

    if (input->is_action_pressed("move_left")) {
        direction -= 1.0f;
    }
    if (input->is_action_pressed("move_right")) {
        direction += 1.0f;
    }

    // 移动
    move(direction);

    // 跳跃输入：直接使用父类的is_on_floor()方法检查是否在地面
    if (input->is_action_just_pressed("jump") && is_on_floor()) {
        jump();
    }

    // 应用物理
    move_and_slide();
}

// 移动实现
void Player::move(float direction) {
    Vector2 velocity = get_velocity();
    velocity.x = direction * move_speed;
    
    // 应用重力（如果需要）
    // 注意：CharacterBody2D的move_and_slide会自动应用重力
    // 但您可能需要手动设置垂直速度
    velocity.y = get_velocity().y; // 保持当前垂直速度
    
    set_velocity(velocity);
}

// 跳跃实现
void Player::jump() {
    Vector2 velocity = get_velocity();
    velocity.y = -jump_force; // Godot 2D 坐标系Y轴向下为正，所以向上跳跃为负
    set_velocity(velocity);
    // 不需要设置is_on_floor状态，由Godot物理系统管理
}

// 属性getter/setter
float Player::get_move_speed() const { return move_speed; }
void Player::set_move_speed(float speed) { move_speed = speed; }
float Player::get_jump_force() const { return jump_force; }
void Player::set_jump_force(float force) { jump_force = force; }

} // namespace godot