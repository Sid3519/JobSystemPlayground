#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight, float near, float far)
{
	m_mode = Mode_Orthographic;
	m_orthographicBotttomLeft = bottomLeft;
	m_orthographicTopRight = topRight;
	m_orthographicNear = near;
	m_orthographicFar = far;
}

void Camera::SetPerspectiveView(float aspect, float fov, float near, float far)
{
	m_mode = Mode_Perspective;
	m_perspectiveAspect = aspect;
	m_perspectiveFOV = fov;
	m_perspectiveNear = near;
	m_perspectiveFar = far;
}

void Camera::SetPerspectiveFOV(float fov)
{
	m_perspectiveFOV = fov;
}

Vec2 Camera::GetCameraCenter() const
{
	AABB2 camBounds(m_orthographicBotttomLeft, m_orthographicTopRight);
	Vec2 currentCenter = camBounds.GetCenter();

	return currentCenter;
}

void Camera::SetCameraCenter(Vec2 const& newCenter)
{
	AABB2 camBounds(m_orthographicBotttomLeft, m_orthographicTopRight);
	camBounds.SetCenter(newCenter);
	m_orthographicBotttomLeft = camBounds.m_mins;
	m_orthographicTopRight = camBounds.m_maxs;
}

void Camera::SetCameraViewport(float minX, float minY, float maxX, float maxY)
{
	m_viewport.m_mins.x = minX;
	m_viewport.m_mins.y = minY;
	m_viewport.m_maxs.x = maxX;
	m_viewport.m_maxs.y = maxY;
}

void Camera::SetCameraViewport(Vec2 mins, Vec2 maxs)
{
	m_viewport.m_mins = mins;
	m_viewport.m_maxs = maxs;
}

void Camera::SetCameraViewport(AABB2 viewport)
{
	m_viewport = viewport;
}

void Camera::SetRenderBasis(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_renderIBasis = iBasis;
	m_renderJBasis = jBasis;
	m_renderKBasis = kBasis;
}

void Camera::SetTransform(Vec3 const& position, EulerAngles const& orientation)
{
	m_position = position;
	m_orientation = orientation;
}

void Camera::SetPositionOnly(Vec3 const& position)
{
	m_position = position;
}

void Camera::SetOrientationOnly(EulerAngles const& orientation)
{
	m_orientation = orientation;
}

Mat44 Camera::GetOrthographicMatrix() const
{
	Vec2 const& BL = m_orthographicBotttomLeft;
	Vec2 const& TR = m_orthographicTopRight;
	Mat44 orthographicMat = Mat44::CreateOrthoProjection(BL.x, TR.x, BL.y, TR.y, m_orthographicNear, m_orthographicFar);
	return orthographicMat;
}

Mat44 Camera::GetPerspectiveMatrix() const
{
	Mat44 perspectiveMat = Mat44::CreatePerspectiveProjection(m_perspectiveFOV, m_perspectiveAspect, m_perspectiveNear, m_perspectiveFar);
	return perspectiveMat;
}

Mat44 Camera::GetProjectionMatrix() const
{
	// Mat44 viewMat = GetViewMatrix();
	Mat44 renderMat = GetRenderMatrix();
	Mat44 projectionMat;
	if (m_mode == Mode_Orthographic)
	{
		projectionMat = GetOrthographicMatrix();
	}
	else if (m_mode == Mode_Perspective)
	{
		projectionMat = GetPerspectiveMatrix();
	}
	else
	{
		ERROR_AND_DIE("Invalid camera mode: " + m_mode);
	}
	//projectionMat.Append(viewMat);
	projectionMat.Append(renderMat);
	return projectionMat;
}

Mat44 Camera::GetRenderMatrix() const
{
	Mat44 renderMat;
	renderMat.SetIJK3D(m_renderIBasis, m_renderJBasis, m_renderKBasis);
	return renderMat;
}

Mat44 Camera::GetViewMatrix() const
{
	Mat44 viewMat = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	viewMat.SetTranslation3D(m_position);

	viewMat = viewMat.GetOrthonormalInverse();
	return viewMat;
}

Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_orthographicBotttomLeft;
}

Vec2 Camera::GetOrthoTopRight() const
{
	return m_orthographicTopRight;
}

AABB2 Camera::GetCameraAABB() const
{
	AABB2 camBounds(m_orthographicBotttomLeft, m_orthographicTopRight);
	return camBounds;
}

AABB2 Camera::GetCameraViewport() const
{
	return m_viewport;
}

Vec3 Camera::GetCameraPosition() const
{
	return m_position;
}

EulerAngles Camera::GetCameraOrientation() const
{
	return m_orientation;
}

void Camera::Translate2D(Vec2 const& translation)
{
	(void)translation;
}

Vec2 Camera::CameraShake(float randomTheta, float shakeAmount)
{
	AABB2 camBounds(m_orthographicBotttomLeft, m_orthographicTopRight);
	Vec2 currentCenter = camBounds.GetCenter();

	// float currentOrientationDegrees = currentCenter.GetOrientationDegrees();

	// TODO(sid): Rename
	Vec2 newCenter = Vec2::MakeFromPolarDegrees(randomTheta, shakeAmount);
	newCenter += currentCenter;

	return newCenter;
}

