#include "PhysicsNode.h"
#include "PhysicsEngine.h"

void PhysicsNode::DeleteAllCollisionShapes() {
	for (auto i = collisionShapes.begin(); i != collisionShapes.end(); ++i) {
		SAFE_DELETE(*i);
	}
	collisionShapes.clear();
	weights = 0;
}

void PhysicsNode::AddCollisionShape(CollisionShape* colShape, Vector3 offset, float weight) {
	if (colShape)
	{
		/*if (collisionShapes.empty())
		{
			position = offset;
			weights = weight;
		}
		else
		{
			position = (position*weights + offset*weight) / (weights + weight);
			weights += weight;
		}*/
		colShape->SetParent(this);
		colShape->SetOffset(offset);
		collisionShapes.push_back(colShape);
	}
}

void PhysicsNode::IntegrateForVelocity(float dt)
{
	if (invMass > 0.0f)
		linVelocity += PhysicsEngine::Instance()->GetGravity() * dt;

	linVelocity += force * invMass * dt;

	linVelocity = linVelocity * PhysicsEngine::Instance()->GetDampingFactor();

	angVelocity += invInertia * torque * dt;

	angVelocity = angVelocity * PhysicsEngine::Instance() -> GetDampingFactor();
}

/* Between these two functions the physics engine will solve for velocity
   based on collisions/constraints etc. So we need to integrate velocity, solve 
   constraints, then use final velocity to update position. 
*/

void PhysicsNode::IntegrateForPosition(float dt)
{
	position += linVelocity * dt;

	orientation = orientation + Quaternion(angVelocity * dt * 0.5f, 0.0f) * orientation;

	orientation.Normalise();

	Vector3 v_min, v_max;

	if (!collisionShapes.empty())
	{
		if (collisionShapes.size() == 1)
		{
			//x axis
			collisionShapes[0]->GetMinMaxVertexOnAxis(Vector3(1, 0, 0), v_min, v_max);
			axisAlignedBoundingBox.x = v_max.x - position.x;

			//y axis
			collisionShapes[0]->GetMinMaxVertexOnAxis(Vector3(0, 1, 0), v_min, v_max);
			axisAlignedBoundingBox.y = v_max.y - position.y;

			//z axis
			collisionShapes[0]->GetMinMaxVertexOnAxis(Vector3(0, 0, 1), v_min, v_max);
			axisAlignedBoundingBox.z = v_max.z - position.z;

			if (subspace)
				subspace->UpdateNode(this);
		}
		else {
			//x axis
			collisionShapes[0]->GetMinMaxVertexOnAxis(Vector3(1, 0, 0), v_min, v_max);
			axisAlignedBoundingBox.x = v_max.x - position.x + collisionShapes[0]->GetOffset().x;

			//y axis
			collisionShapes[0]->GetMinMaxVertexOnAxis(Vector3(0, 1, 0), v_min, v_max);
			axisAlignedBoundingBox.y = v_max.y - position.y + collisionShapes[0]->GetOffset().y;

			//z axis
			collisionShapes[0]->GetMinMaxVertexOnAxis(Vector3(0, 0, 1), v_min, v_max);
			axisAlignedBoundingBox.z = v_max.z - position.z + collisionShapes[0]->GetOffset().z;

			for (auto i = collisionShapes.begin() + 1; i != collisionShapes.end(); ++i)
			{
				float temp;
				//x
				(*i)->GetMinMaxVertexOnAxis(Vector3(1, 0, 0), v_min, v_max);
				temp = v_max.x - position.x + collisionShapes[0]->GetOffset().x;
				if (temp > axisAlignedBoundingBox.x)
				{
					axisAlignedBoundingBox.x = temp;
				}

				//y
				(*i)->GetMinMaxVertexOnAxis(Vector3(0, 1, 0), v_min, v_max);
				temp = v_max.y - position.y + collisionShapes[0]->GetOffset().y;
				if (temp > axisAlignedBoundingBox.y)
				{
					axisAlignedBoundingBox.y = temp;
				}

				//z
				(*i)->GetMinMaxVertexOnAxis(Vector3(0, 0, 1), v_min, v_max);
				temp = v_max.z - position.z + collisionShapes[0]->GetOffset().z;
				if (temp > axisAlignedBoundingBox.z)
				{
					axisAlignedBoundingBox.z = temp;
				}
			}
		}
	}

	//Finally: Notify any listener's that this PhysicsNode has a new world transform.
	// - This is used by GameObject to set the worldTransform of any RenderNode's. 
	//   Please don't delete this!!!!!
	FireOnUpdateCallback();
}

void PhysicsNode::SetPosition(const Vector3& v) {
	position = v;
	if(subspace)
		subspace->UpdateNode(this);
	FireOnUpdateCallback();
}

void PhysicsNode::DebugDrawAABB() {
	Vector3 p, a;
	p = position;
	a = axisAlignedBoundingBox;
	NCLDebug::DrawThickLineNDT(Vector3(p.x - a.x, p.y - a.y, p.z + a.z), Vector3(p.x + a.x, p.y - a.y, p.z + a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x - a.x, p.y - a.y, p.z + a.z), Vector3(p.x - a.x, p.y + a.y, p.z + a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x + a.x, p.y + a.y, p.z + a.z), Vector3(p.x + a.x, p.y - a.y, p.z + a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x + a.x, p.y + a.y, p.z + a.z), Vector3(p.x - a.x, p.y + a.y, p.z + a.z), 0.05f, Vector4(1, 0, 0, 1));

	NCLDebug::DrawThickLineNDT(Vector3(p.x - a.x, p.y - a.y, p.z - a.z), Vector3(p.x + a.x, p.y - a.y, p.z - a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x - a.x, p.y - a.y, p.z - a.z), Vector3(p.x - a.x, p.y + a.y, p.z - a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x + a.x, p.y + a.y, p.z - a.z), Vector3(p.x + a.x, p.y - a.y, p.z - a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x + a.x, p.y + a.y, p.z - a.z), Vector3(p.x - a.x, p.y + a.y, p.z - a.z), 0.05f, Vector4(1, 0, 0, 1));

	NCLDebug::DrawThickLineNDT(Vector3(p.x + a.x, p.y + a.y, p.z + a.z), Vector3(p.x + a.x, p.y + a.y, p.z - a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x + a.x, p.y - a.y, p.z + a.z), Vector3(p.x + a.x, p.y - a.y, p.z - a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x - a.x, p.y + a.y, p.z + a.z), Vector3(p.x - a.x, p.y + a.y, p.z - a.z), 0.05f, Vector4(1, 0, 0, 1));
	NCLDebug::DrawThickLineNDT(Vector3(p.x - a.x, p.y - a.y, p.z + a.z), Vector3(p.x - a.x, p.y - a.y, p.z - a.z), 0.05f, Vector4(1, 0, 0, 1));
}