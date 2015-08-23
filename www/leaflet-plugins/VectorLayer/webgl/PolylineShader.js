//******************************************************************************
//
// File Name : PolylineShader.js
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

function PolylineShader(options) {
}

PolylineShader.prototype.restore = function(gl) 
{
	this.gl = gl;

	/* POLYLINE SHADER PROGRAM */
	var polylineDrawVertexShader = gl.createShader(gl.VERTEX_SHADER);
	this.polylineDrawVertexShader = polylineDrawVertexShader;
	
	gl.shaderSource(polylineDrawVertexShader,	"\
									precision mediump float;\
									attribute vec2 a_position;\
									attribute vec2 a_normal;\
									attribute vec2 a_textureCoordinate;\
									uniform float u_line_width;\
									uniform mat4 u_mv_matrix;\
									uniform mat4 u_proj_matrix;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										gl_Position = u_proj_matrix * (u_mv_matrix * vec4(a_position, 0, 1) + vec4(a_normal, 0, 0) * u_line_width);\
										v_textureCoordinate = a_textureCoordinate;\
									}");

	gl.compileShader(polylineDrawVertexShader);

	if (!gl.getShaderParameter(polylineDrawVertexShader, gl.COMPILE_STATUS)) {
		console.log('polylineDrawVertexShader: ' + gl.getShaderInfoLog(polylineDrawVertexShader));
		return -1;
	}

	/* Fragment shader */
	var polylineDrawFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	this.polylineDrawFragmentShader = polylineDrawFragmentShader;

	gl.shaderSource(polylineDrawFragmentShader, "\
									precision mediump float;\
									uniform sampler2D u_sampler;\
									uniform float u_frame_number;\
									uniform float u_frame_length;\
									uniform float u_mask;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										vec2 tex = v_textureCoordinate;\
										tex.t -= mod(u_frame_number, u_frame_length) / u_frame_length;\
										gl_FragColor = (u_mask == 0.0) ? texture2D( u_sampler, tex ) : vec4(1.0);\
									}");

	gl.compileShader(polylineDrawFragmentShader);

	if (!gl.getShaderParameter(polylineDrawFragmentShader, gl.COMPILE_STATUS)) {
		console.log('polylineDrawFragmentShader: ' + gl.getShaderInfoLog(polylineDrawFragmentShader));
		return -1;
	}

	/* Shader program */
	var polylineDrawShaderProgram = gl.createProgram();
	this.polylineDrawShaderProgram = polylineDrawShaderProgram;

	gl.attachShader(polylineDrawShaderProgram, polylineDrawVertexShader);
	gl.attachShader(polylineDrawShaderProgram, polylineDrawFragmentShader);
	gl.linkProgram(polylineDrawShaderProgram);

	gl.useProgram(polylineDrawShaderProgram);

	polylineDrawShaderProgram.posAttrib					= gl.getAttribLocation(polylineDrawShaderProgram,  "a_position");
	polylineDrawShaderProgram.normalAttrib				= gl.getAttribLocation(polylineDrawShaderProgram,  "a_normal");
	polylineDrawShaderProgram.textureAttrib				= gl.getAttribLocation(polylineDrawShaderProgram,  "a_textureCoordinate");
	polylineDrawShaderProgram.modelviewMatrixUniform	= gl.getUniformLocation(polylineDrawShaderProgram, "u_mv_matrix");
	polylineDrawShaderProgram.projMatrixUniform			= gl.getUniformLocation(polylineDrawShaderProgram, "u_proj_matrix");
	polylineDrawShaderProgram.lineWidthUniform			= gl.getUniformLocation(polylineDrawShaderProgram, "u_line_width");
	polylineDrawShaderProgram.samplerUniform			= gl.getUniformLocation(polylineDrawShaderProgram, "u_sampler");
	polylineDrawShaderProgram.frameNumberUniform		= gl.getUniformLocation(polylineDrawShaderProgram, "u_frame_number");
	polylineDrawShaderProgram.frameLengthUniform		= gl.getUniformLocation(polylineDrawShaderProgram, "u_frame_length");
	polylineDrawShaderProgram.maskUniform				= gl.getUniformLocation(polylineDrawShaderProgram, "u_mask");

	var spriteDrawVertexShader = gl.createShader(gl.VERTEX_SHADER);
	this.spriteDrawVertexShader = spriteDrawVertexShader;
	
	gl.shaderSource(spriteDrawVertexShader,	"\
									precision mediump float;\
									attribute vec2 a_position;\
									attribute vec2 a_normal;\
									attribute vec2 a_textureCoordinate;\
									uniform mat4 u_mv_matrix;\
									uniform mat4 u_proj_matrix;\
									uniform float u_point_size;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										gl_Position = u_proj_matrix * (u_mv_matrix * vec4(a_position, 0, 1) + vec4(a_normal, 0, 0) * u_point_size);\
										v_textureCoordinate = a_textureCoordinate;\
									}");

	gl.compileShader(spriteDrawVertexShader);

	if (!gl.getShaderParameter(spriteDrawVertexShader, gl.COMPILE_STATUS)) {
		console.log('spriteDrawVertexShader: ' + gl.getShaderInfoLog(spriteDrawVertexShader));
	}

	var spriteDrawFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	this.spriteDrawFragmentShader = spriteDrawFragmentShader;

	gl.shaderSource(spriteDrawFragmentShader, "\
									precision mediump float;\
									uniform sampler2D u_sampler;\
									varying vec2 v_textureCoordinate;\
									void main(void) {\
										gl_FragColor = texture2D(u_sampler, v_textureCoordinate);\
									}");

	gl.compileShader(spriteDrawFragmentShader);

	if (!gl.getShaderParameter(spriteDrawFragmentShader, gl.COMPILE_STATUS)) {
		console.log('spriteDrawFragmentShader: ' + gl.getShaderInfoLog(spriteDrawFragmentShader));
	}

	var spriteDrawShaderProgram = gl.createProgram();
	this.spriteDrawShaderProgram = spriteDrawShaderProgram;

	gl.attachShader(spriteDrawShaderProgram, spriteDrawVertexShader);
	gl.attachShader(spriteDrawShaderProgram, spriteDrawFragmentShader);
	gl.linkProgram(spriteDrawShaderProgram);

	gl.useProgram(spriteDrawShaderProgram);
	
	spriteDrawShaderProgram.posAttrib				= gl.getAttribLocation(spriteDrawShaderProgram,  "a_position");
	spriteDrawShaderProgram.normalAttrib			= gl.getAttribLocation(spriteDrawShaderProgram,  "a_normal");
	spriteDrawShaderProgram.textureAttrib			= gl.getAttribLocation(spriteDrawShaderProgram,  "a_textureCoordinate");
	spriteDrawShaderProgram.modelviewMatrixUniform	= gl.getUniformLocation(spriteDrawShaderProgram, "u_mv_matrix");
	spriteDrawShaderProgram.projMatrixUniform		= gl.getUniformLocation(spriteDrawShaderProgram, "u_proj_matrix");
	spriteDrawShaderProgram.pointSizeUniform		= gl.getUniformLocation(spriteDrawShaderProgram, "u_point_size");
	spriteDrawShaderProgram.samplerUniform			= gl.getUniformLocation(spriteDrawShaderProgram, "u_sampler");

	var polylineSelectVertexShader = gl.createShader(gl.VERTEX_SHADER);
	this.polylineSelectVertexShader = polylineSelectVertexShader;
	
	gl.shaderSource(polylineSelectVertexShader,	"\
									precision mediump float;\
									attribute vec2 a_position;\
									attribute vec2 a_normal;\
									uniform float u_line_width;\
									uniform mat4 u_mv_matrix;\
									uniform mat4 u_proj_matrix;\
									void main(void) {\
										gl_Position = u_proj_matrix * (u_mv_matrix * vec4(a_position, 0, 1) + vec4(a_normal, 0, 0) * u_line_width);\
									}");

	gl.compileShader(polylineSelectVertexShader);

	if (!gl.getShaderParameter(polylineSelectVertexShader, gl.COMPILE_STATUS)) {
		console.log('polylineSelectVertexShader: ' + gl.getShaderInfoLog(polylineSelectVertexShader));
	}

	/* Fragment shader */
	var polylineSelectFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	this.polylineSelectFragmentShader = polylineSelectFragmentShader;

	gl.shaderSource(polylineSelectFragmentShader, "\
									precision mediump float;\
									void main(void) {\
										gl_FragColor = vec4(1.0);\
									}");

	gl.compileShader(polylineSelectFragmentShader);

	if (!gl.getShaderParameter(polylineSelectFragmentShader, gl.COMPILE_STATUS)) {
		console.log('polylineSelectFragmentShader: ' + gl.getShaderInfoLog(polylineSelectFragmentShader));
	}

	/* Shader program */
	var polylineSelectShaderProgram = gl.createProgram();
	this.polylineSelectShaderProgram = polylineSelectShaderProgram;

	gl.attachShader(polylineSelectShaderProgram, polylineSelectVertexShader);
	gl.attachShader(polylineSelectShaderProgram, polylineSelectFragmentShader);
	gl.linkProgram(polylineSelectShaderProgram);

	gl.useProgram(polylineSelectShaderProgram);

	polylineSelectShaderProgram.posAttrib				= gl.getAttribLocation(polylineSelectShaderProgram,  "a_position");
	polylineSelectShaderProgram.normalAttrib			= gl.getAttribLocation(polylineSelectShaderProgram,  "a_normal");
	polylineSelectShaderProgram.modelviewMatrixUniform	= gl.getUniformLocation(polylineSelectShaderProgram, "u_mv_matrix");
	polylineSelectShaderProgram.projMatrixUniform		= gl.getUniformLocation(polylineSelectShaderProgram, "u_proj_matrix");
	polylineSelectShaderProgram.lineWidthUniform		= gl.getUniformLocation(polylineSelectShaderProgram, "u_line_width");

	var spriteSelectVertexShader = gl.createShader(gl.VERTEX_SHADER);
	this.spriteSelectVertexShader = spriteSelectVertexShader;
	
	gl.shaderSource(spriteSelectVertexShader,	"\
									precision mediump float;\
									attribute vec2 a_position;\
									attribute vec2 a_normal;\
									attribute vec2 a_id;\
									uniform mat4 u_mv_matrix;\
									uniform mat4 u_proj_matrix;\
									uniform float u_point_size;\
									varying vec2 v_id;\
									void main(void) {\
										gl_Position = u_proj_matrix * (u_mv_matrix * vec4(a_position, 0, 1) + vec4(a_normal, 0, 0) * u_point_size);\
										v_id = a_id;\
									}");

	gl.compileShader(spriteSelectVertexShader);

	if (!gl.getShaderParameter(spriteSelectVertexShader, gl.COMPILE_STATUS)) {
		console.log('spriteSelectVertexShader: ' + gl.getShaderInfoLog(spriteSelectVertexShader));
	}

	var spriteSelectFragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
	this.spriteSelectFragmentShader = spriteSelectFragmentShader;

	gl.shaderSource(spriteSelectFragmentShader, "\
									precision mediump float;\
									varying vec2 v_id;\
									void main(void) {\
										gl_FragColor = vec4(v_id, 0.0, 1.0);\
									}");

	gl.compileShader(spriteSelectFragmentShader);

	if (!gl.getShaderParameter(spriteSelectFragmentShader, gl.COMPILE_STATUS)) {
		console.log('spriteSelectFragmentShader: ' + gl.getShaderInfoLog(spriteSelectFragmentShader));
	}

	var spriteSelectShaderProgram = gl.createProgram();
	this.spriteSelectShaderProgram = spriteSelectShaderProgram;

	gl.attachShader(spriteSelectShaderProgram, spriteSelectVertexShader);
	gl.attachShader(spriteSelectShaderProgram, spriteSelectFragmentShader);
	gl.linkProgram(spriteSelectShaderProgram);

	gl.useProgram(spriteSelectShaderProgram);
	
	spriteSelectShaderProgram.posAttrib					= gl.getAttribLocation(spriteSelectShaderProgram,  "a_position");
	spriteSelectShaderProgram.normalAttrib				= gl.getAttribLocation(spriteSelectShaderProgram,  "a_normal");
	spriteSelectShaderProgram.idAttrib					= gl.getAttribLocation(spriteSelectShaderProgram,  "a_id");
	spriteSelectShaderProgram.modelviewMatrixUniform	= gl.getUniformLocation(spriteSelectShaderProgram, "u_mv_matrix");
	spriteSelectShaderProgram.projMatrixUniform			= gl.getUniformLocation(spriteSelectShaderProgram, "u_proj_matrix");
	spriteSelectShaderProgram.pointSizeUniform			= gl.getUniformLocation(spriteSelectShaderProgram, "u_point_size");
};

PolylineShader.prototype.lost = function() 
{
	this.gl = null;
};

PolylineShader.prototype.cleanup = function() 
{	
	var gl = this.gl;
	             
	if (gl === null)
		return;

	gl.deleteProgram(this.polylineDrawShaderProgram);

	gl.deleteShader(this.polylineDrawVertexShader);
	gl.deleteShader(this.polylineDrawFragmentShader);

	gl.deleteProgram(this.spriteDrawShaderProgram);

	gl.deleteShader(this.spriteDrawVertexShader);
	gl.deleteShader(this.spriteDrawFragmentShader);

	gl.deleteProgram(this.polylineSelectShaderProgram);

	gl.deleteShader(this.polylineSelectVertexShader);
	gl.deleteShader(this.polylineSelectFragmentShader);

	gl.deleteProgram(this.spriteSelectShaderProgram);

	gl.deleteShader(this.spriteSelectVertexShader);
	gl.deleteShader(this.spriteSelectFragmentShader);
};