#ifndef PLAYER_H
#define PLAYER_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

class Player : public CharacterBody2D {
    GDCLASS(Player, CharacterBody2D)  // Godot 反射宏

private:
    // 属性：移动速度、跳跃力
    float move_speed;
    float jump_force;

protected:
    // 声明Godot的属性绑定和方法注册
    static void _bind_methods();

public:
    Player();
    ~Player();

    // Godot 生命周期函数
    void _ready() override;
    void _physics_process(double delta) override;

    // 自定义方法
    void move(float direction);
    void jump();
    // 移除了 handle_collision 方法，因为Godot物理系统已处理

    // 属性getter/setter
    float get_move_speed() const;
    void set_move_speed(float speed);
    float get_jump_force() const;
    void set_jump_force(float force);
};

} // namespace godot

#endif // PLAYER_H