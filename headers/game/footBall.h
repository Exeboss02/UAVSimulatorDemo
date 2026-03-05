#include "core/physics/rigidBody.h"
#include "gameObjects/meshObject.h"

class FootBall : public RigidBody
{
public:
    FootBall();
    ~FootBall();

    void Start() override;
    void Tick() override;

private:
};
