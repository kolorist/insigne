/*
 * this is auto generated code, please do not modify
 * generated by codegen.exe
 */
 
#include <clover.h>

#include "insigne/generated_code/proxy.h"

namespace insigne {

void assert_driver_no_error()
{
	GLenum error = glGetError();
	switch (error) {
		case GL_INVALID_ENUM:
			CLOVER_DEBUG("Error in GraphicsDriver: GL_INVALID_ENUM");
			break;
		case GL_INVALID_VALUE:
			CLOVER_DEBUG("Error in GraphicsDriver: GL_INVALID_VALUE");
			break;
		case GL_INVALID_OPERATION:
			CLOVER_DEBUG("Error in GraphicsDriver: GL_INVALID_OPERATION");
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			CLOVER_DEBUG("Error in GraphicsDriver: GL_INVALID_FRAMEBUFFER_OPERATION");
			break;
		case GL_OUT_OF_MEMORY:
			CLOVER_DEBUG("Error in GraphicsDriver: GL_OUT_OF_MEMORY");
			break;
		case GL_NO_ERROR:
			break;
		default:
			CLOVER_DEBUG("Something wrong with the driver!");
			break;
	}
	if (error != GL_NO_ERROR) {
		volatile int* ptr = nullptr;
		*ptr = 10;
	}
}

void assert_framebuffer_completed()
{
	GLenum status = pxCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	switch (status) {
		case GL_FRAMEBUFFER_UNDEFINED:
			CLOVER_DEBUG("Error in Framebuffer: GL_FRAMEBUFFER_UNDEFINED\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			CLOVER_DEBUG("Error in Framebuffer: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			CLOVER_DEBUG("Error in Framebuffer: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			CLOVER_DEBUG("Error in Framebuffer: GL_FRAMEBUFFER_UNSUPPORTED\n");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			CLOVER_DEBUG("Error in Framebuffer: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n");
			break;
		case GL_FRAMEBUFFER_COMPLETE:
			break;
		default:
			CLOVER_DEBUG("Some error happened with Framebuffer\n");
	}
}

void pxActiveTexture(GLenum texture)
{
	glActiveTexture(texture);
	assert_driver_no_error();
}

void pxClearDepthf(GLfloat depth)
{
	glClearDepthf(depth);
	assert_driver_no_error();
}

void pxColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	glColorMask(red, green, blue, alpha);
	assert_driver_no_error();
}

void pxAttachShader(GLuint program, GLuint shader)
{
	glAttachShader(program, shader);
	assert_driver_no_error();
}

void pxBindBuffer(GLenum target, GLuint buffer)
{
	glBindBuffer(target, buffer);
	assert_driver_no_error();
}

void pxBindFramebuffer(GLenum target, GLuint framebuffer)
{
	glBindFramebuffer(target, framebuffer);
	assert_driver_no_error();
}

void pxBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
	glBindRenderbuffer(target, renderbuffer);
	assert_driver_no_error();
}

void pxBindTexture(GLenum target, GLuint texture)
{
	glBindTexture(target, texture);
	assert_driver_no_error();
}

void pxBindVertexArray(GLuint targetArray)
{
	glBindVertexArray(targetArray);
	assert_driver_no_error();
}

void pxBlendEquation(GLenum mode)
{
	glBlendEquation(mode);
	assert_driver_no_error();
}

void pxBlendFunc(GLenum sfactor, GLenum dfactor)
{
	glBlendFunc(sfactor, dfactor);
	assert_driver_no_error();
}

void pxBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
	glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
	assert_driver_no_error();
}

void pxBufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
	glBufferData(target, size, data, usage);
	assert_driver_no_error();
}

void pxBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data)
{
	glBufferSubData(target, offset, size, data);
	assert_driver_no_error();
}

GLenum pxCheckFramebufferStatus(GLenum target)
{
	GLenum ret = glCheckFramebufferStatus(target);
	assert_driver_no_error();
	return ret;
}

void pxClear(GLbitfield mask)
{
	glClear(mask);
	assert_driver_no_error();
}

void pxClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glClearColor(red, green, blue, alpha);
	assert_driver_no_error();
}

void pxCompileShader(GLuint shader)
{
	glCompileShader(shader);
	assert_driver_no_error();
}

GLuint pxCreateProgram()
{
	GLuint ret = glCreateProgram();
	assert_driver_no_error();
	return ret;
}

GLuint pxCreateShader(GLenum shaderType)
{
	GLuint ret = glCreateShader(shaderType);
	assert_driver_no_error();
	return ret;
}

void pxCullFace(GLenum mode)
{
	glCullFace(mode);
	assert_driver_no_error();
}

void pxDeleteBuffers(GLsizei n, const GLuint* buffers)
{
	glDeleteBuffers(n, buffers);
	assert_driver_no_error();
}

void pxDeleteShader(GLuint shader)
{
	glDeleteShader(shader);
	assert_driver_no_error();
}

void pxDeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
	glDeleteVertexArrays(n, arrays);
	assert_driver_no_error();
}

void pxDepthFunc(GLenum func)
{
	glDepthFunc(func);
	assert_driver_no_error();
}

void pxDepthMask(GLboolean flag)
{
	glDepthMask(flag);
	assert_driver_no_error();
}

void pxDisable(GLenum cap)
{
	glDisable(cap);
	assert_driver_no_error();
}

void pxDrawBuffers(GLsizei n, const GLenum* bufs)
{
	glDrawBuffers(n, bufs);
	assert_driver_no_error();
}

void pxDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
	glDrawElements(mode, count, type, indices);
	assert_driver_no_error();
}

void pxEnable(GLenum cap)
{
	glEnable(cap);
	assert_driver_no_error();
}

void pxEnableVertexAttribArray(GLuint index)
{
	glEnableVertexAttribArray(index);
	assert_driver_no_error();
}

void pxFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
	glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
	assert_driver_no_error();
}

void pxFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
	glFramebufferTexture2D(target, attachment, textarget, texture, level);
	assert_driver_no_error();
}

void pxFrontFace(GLenum mode)
{
	glFrontFace(mode);
	assert_driver_no_error();
}

void pxGenBuffers(GLsizei n, GLuint* buffers)
{
	glGenBuffers(n, buffers);
	assert_driver_no_error();
}

void pxGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
	glGenFramebuffers(n, framebuffers);
	assert_driver_no_error();
}

void pxGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
	glGenRenderbuffers(n, renderbuffers);
	assert_driver_no_error();
}

void pxGenTextures(GLsizei n, GLuint* textures)
{
	glGenTextures(n, textures);
	assert_driver_no_error();
}

void pxGenVertexArrays(GLsizei n, GLuint* arrays)
{
	glGenVertexArrays(n, arrays);
	assert_driver_no_error();
}

void pxGenerateMipmap(GLenum target)
{
	glGenerateMipmap(target);
	assert_driver_no_error();
}

GLint pxGetAttribLocation(GLuint program, const GLchar* name)
{
	GLint ret = glGetAttribLocation(program, name);
	assert_driver_no_error();
	return ret;
}

GLenum pxGetError()
{
	GLenum ret = glGetError();
	assert_driver_no_error();
	return ret;
}

void pxGetIntegerv(GLenum pname, GLint* data)
{
	glGetIntegerv(pname, data);
	assert_driver_no_error();
}

void pxGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog)
{
	glGetShaderInfoLog(shader, maxLength, length, infoLog);
	assert_driver_no_error();
}

void pxGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
	glGetShaderiv(shader, pname, params);
	assert_driver_no_error();
}

const GLubyte* pxGetString(GLenum name)
{
	const GLubyte* ret = glGetString(name);
	assert_driver_no_error();
	return ret;
}

const GLubyte* pxGetStringi(GLenum name, GLuint index)
{
	const GLubyte* ret = glGetStringi(name, index);
	assert_driver_no_error();
	return ret;
}

GLint pxGetUniformLocation(GLuint program, const GLchar* name)
{
	GLint ret = glGetUniformLocation(program, name);
	assert_driver_no_error();
	return ret;
}

void pxLinkProgram(GLuint program)
{
	glLinkProgram(program);
	assert_driver_no_error();
}

void pxPixelStorei(GLenum pname, GLint param)
{
	glPixelStorei(pname, param);
	assert_driver_no_error();
}

void pxRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
	glRenderbufferStorage(target, internalformat, width, height);
	assert_driver_no_error();
}

void pxRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height)
{
	glRenderbufferStorageMultisample(target, samples, internalFormat, width, height);
	assert_driver_no_error();
}

void pxScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	glScissor(x, y, width, height);
	assert_driver_no_error();
}

void pxShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
	glShaderSource(shader, count, string, length);
	assert_driver_no_error();
}

void pxStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	glStencilFunc(func, ref, mask);
	assert_driver_no_error();
}

void pxStencilOp(GLenum sfail, GLenum dpfail, GLenum dppass)
{
	glStencilOp(sfail, dpfail, dppass);
	assert_driver_no_error();
}

void pxTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* data)
{
	glTexImage2D(target, level, internalFormat, width, height, border, format, type, data);
	assert_driver_no_error();
}

void pxTexStorage2D(GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height)
{
	glTexStorage2D(target, levels, internalFormat, width, height);
	assert_driver_no_error();
}

void pxTexParameteri(GLenum target, GLenum pname, GLint param)
{
	glTexParameteri(target, pname, param);
	assert_driver_no_error();
}

void pxUniform1f(GLint location, GLfloat v0)
{
	glUniform1f(location, v0);
	assert_driver_no_error();
}

void pxUniform1i(GLint location, GLint v0)
{
	glUniform1i(location, v0);
	assert_driver_no_error();
}

void pxUniform3fv(GLint location, GLsizei count, const GLfloat* value)
{
	glUniform3fv(location, count, value);
	assert_driver_no_error();
}

void pxUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
	glUniformMatrix4fv(location, count, transpose, value);
	assert_driver_no_error();
}

void pxUseProgram(GLuint program)
{
	glUseProgram(program);
	assert_driver_no_error();
}

void pxVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
{
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	assert_driver_no_error();
}

void pxViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	glViewport(x, y, width, height);
	assert_driver_no_error();
}

GLuint pxGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
	GLuint ret = glGetUniformBlockIndex(program, uniformBlockName);
	assert_driver_no_error();
	return ret;
}

void pxBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
	glBindBufferBase(target, index, buffer);
	assert_driver_no_error();
}

void pxBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
	glBindBufferRange(target, index, buffer, offset, size);
	assert_driver_no_error();
}

void pxUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
	glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
	assert_driver_no_error();
}

void pxReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * data)
{
	glReadPixels(x, y, width, height, format, type, data);
	assert_driver_no_error();
}

void pxReadBuffer(GLenum src)
{
	glReadBuffer(src);
	assert_driver_no_error();
}

}
