#pragma once
#include <cstdint>

class Health {
public:
    explicit Health(int max = 100) : maxHealth(max), currentHealth(max) {}

    void Decrement(int hp) {
        currentHealth -= hp;
        if (currentHealth < 0) currentHealth = 0;
    }

    void Increment(int hp) {
        currentHealth += hp;
        if (currentHealth > maxHealth) currentHealth = maxHealth;
    }

    int Get() const { return currentHealth; }
    int GetMax() const { return maxHealth; }
    void SetMax(int m) { maxHealth = m; if (currentHealth > maxHealth) currentHealth = maxHealth; }
    bool IsDead() const { return currentHealth <= 0; }

private:
    int maxHealth;
    int currentHealth;
};
