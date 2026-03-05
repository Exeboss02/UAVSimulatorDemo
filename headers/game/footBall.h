#include "core/physics/rigidBody.h"
#include "gameObjects/meshObject.h"

class FootBall : public RigidBody
{
public:
    FootBall();
    ~FootBall();

    void Start() override;
    void Tick() override;
    void OnCollision(std::weak_ptr<GameObject3D> gameObject3D, std::weak_ptr<Collider> collider);

private:
};
