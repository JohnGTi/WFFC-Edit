#include "DisplayObject.h"

#include "directxmath.h"

using namespace DirectX;


DisplayObject::DisplayObject()
{
	m_model = NULL;
	m_texture_diffuse = NULL;
	m_orientation.x = 0.0f;
	m_orientation.y = 0.0f;
	m_orientation.z = 0.0f;
	m_position.x = 0.0f;
	m_position.y = 0.0f;
	m_position.z = 0.0f;
	m_scale.x = 0.0f;
	m_scale.y = 0.0f;
	m_scale.z = 0.0f;
	m_render = true;
	m_wireframe = false;

	m_light_type =0;
	m_light_diffuse_r = 0.0f;	m_light_diffuse_g = 0.0f;	m_light_diffuse_b = 0.0f;
	m_light_specular_r = 0.0f;	m_light_specular_g = 0.0f;	m_light_specular_b = 0.0f;
	m_light_spot_cutoff = 0.0f;
	m_light_constant = 0.0f;
	m_light_linear = 0.0f;
	m_light_quadratic = 0.0f;
}

DisplayObject::~DisplayObject()
{
//	delete m_texture_diffuse;
}


SimpleMath::Matrix DisplayObject::GetGlobalTransformation(SimpleMath::Matrix World)
{
	// Calculate the local transformation matrix of the object.
	
	// Begin with the scale factor and object translation.

	const XMVECTORF32 Scale = { m_scale.x, m_scale.y, m_scale.z };

	const XMVECTORF32 Translate = { m_position.x, m_position.y, m_position.z };

	// Retrieve a quaternion from the yaw/pitch/roll, eular angles.

	const XMVECTOR Rotation = SimpleMath::Quaternion::CreateFromYawPitchRoll(XMConvertToRadians(m_orientation.y)
		, XMConvertToRadians(m_orientation.x)
		, XMConvertToRadians(m_orientation.z));

	// Compose the matrix transformatin in local-space.

	XMMATRIX LocalTransform = XMMatrixTransformation(g_XMZero, SimpleMath::Quaternion::Identity, Scale, g_XMZero, Rotation, Translate);

	// Return the world-space transformation.

	return World * LocalTransform;
}
