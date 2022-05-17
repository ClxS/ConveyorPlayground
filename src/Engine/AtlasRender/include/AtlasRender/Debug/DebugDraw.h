#pragma once

#include <bgfx/bgfx.h>
#include <bx/allocator.h>
#include <bx/bounds.h>

namespace atlas::render::debug
{
    void initialise();

    void shutdown();

    class DebugDrawEncoderScopePush
    {
    public:
        DebugDrawEncoderScopePush();
        ~DebugDrawEncoderScopePush();
    };

    namespace debug_draw
    {
        void begin(uint16_t _viewId, bool _depthTestLess = true, bgfx::Encoder* _encoder = NULL);

        void end();

        void push();

        void pop();

        void setDepthTestLess(bool _depthTestLess);

        void setState(bool _depthTest, bool _depthWrite, bool _clockwise);

        void setColor(uint32_t _abgr);

        void setLod(uint8_t _lod);

        void setWireframe(bool _wireframe);

        void setStipple(bool _stipple, float _scale = 1.0f, float _offset = 0.0f);

        void setSpin(float _spin);

        void setTransform(const void* _mtx);

        void setTranslate(float _x, float _y, float _z);

        void pushTransform(const void* _mtx);

        void popTransform();

        void moveTo(float _x, float _y, float _z = 0.0f);

        void moveTo(const bx::Vec3& _pos);

        void lineTo(float _x, float _y, float _z = 0.0f);

        void lineTo(const bx::Vec3& _pos);

        void close();

        void draw(const bx::Aabb& _aabb);

        void draw(const bx::Cylinder& _cylinder);

        void draw(const bx::Capsule& _capsule);

        void draw(const bx::Disk& _disk);

        void draw(const bx::Obb& _obb);

        void draw(const bx::Sphere& _sphere);

        void draw(const bx::Triangle& _triangle);

        void draw(const bx::Cone& _cone);

        void drawFrustum(const void* _viewProj);

        void drawAxis(float _x, float _y, float _z, float _len = 1.0f, float _thickness = 0.0f);

        void drawCircle(const bx::Vec3& _normal, const bx::Vec3& _center, float _radius, float _weight = 0.0f);

        void drawQuad(const bx::Vec3& _normal, const bx::Vec3& _center, float _size);

        void drawQuad(bgfx::TextureHandle _handle, const bx::Vec3& _normal, const bx::Vec3& _center, float _size);

        void drawCone(const bx::Vec3& _from, const bx::Vec3& _to, float _radius);

        void drawCylinder(const bx::Vec3& _from, const bx::Vec3& _to, float _radius);

        void drawCapsule(const bx::Vec3& _from, const bx::Vec3& _to, float _radius);

        void drawGrid(const bx::Vec3& _normal, const bx::Vec3& _center, uint32_t _size = 20, float _step = 1.0f);

        DebugDrawEncoderScopePush createScope();
    }
}
