#pragma once

/* 
 *  Simple trackball-like motion adapted (ripped off) from projtex.c
 *  (written by David Yu and David Blythe).  See the SIGGRAPH '96
 *  Advanced OpenGL course notes.
 *
 *  Typical setup:
 *
 *
    void
    init(void)
    {
      tbInit(GLUT_MIDDLE_BUTTON);
      tbAnimate(GL_TRUE);
      . . .
    }

    void
    reshape(int width, int height)
    {
      tbReshape(width, height);
      . . .
    }

    void
    display(void)
    {
      glPushMatrix();

      tbMatrix();
      . . . draw the scene . . .

      glPopMatrix();
    }

    void
    mouse(int button, int state, int x, int y)
    {
      tbMouse(button, state, x, y);
      . . .
    }

    void
    motion(int x, int y)
    {
      tbMotion(x, y);
      . . .
    }

    int
    main(int argc, char** argv)
    {
      . . .
      init();
      glutReshapeFunc(reshape);
      glutDisplayFunc(display);
      glutMouseFunc(mouse);
      glutMotionFunc(motion);
      . . .
    }
 *
 * */

#include "MyVec.h"
#include "MyMatrix.h"

class MyTrackBall
{
public:
	MyTrackBall(void);
	~MyTrackBall(void);

	/* functions */

	MyMatrixf Matrix() const;

	void Reshape(int width, int height);

	// mouse interaction interface
	void StartMotion(int x, int y);

	void EndMotion(int x, int y);

	void RotateMotion(int x, int y);

	void TranslateMotion(int x, int y);

	void TranslateMotionX(int x, int y);

	void ScaleMotion(int x, int y);

	float GetScaleFromMotion(int x, int y) const;

	void SetRotationMatrix(const MyMatrixf& mat);

	// others
	// rotation center
	void SetOrigin(const MyVec3f& origin);

	void ResetTranslate();
	void ResetScale();
	void Translate(const MyVec3f& offset);
	void ScaleAdd(float dy);
	void ScaleMultiply(float dy);

protected:
	void pointToVector(int x, int y, int width, int height, MyVec3f& v) const;
	void translate(float dx, float dy);

	float mScale;
	float mButtonX;
	float mButtonY;
	float mAngle;

	MyVec3f mLastPos;
	MyVec3f mAxis;
	MyVec3f mOrigin;

	// 4 by 4 matrix
	MyMatrixf mTranslateMatrix;
	MyMatrixf mRotateMatrix;
	MyMatrixf mScaleMatrix;

	int mWidth;
	int mHeight;

	bool mTracking;
};

