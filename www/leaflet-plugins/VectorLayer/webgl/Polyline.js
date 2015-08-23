//******************************************************************************
//
// File Name : Polyline.js
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

function PolylineGL(options) {

	this.gl = null;
	this.polyline_shader = options.layer.renderer.polyline_shader;
	this.options = options;
	this.sprite_texture_data = null;
	this.polyline_texture_data = null;

    if (typeof options.color != 'undefined') {
		
		var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(options.color);

		if (result) {
			options.color = [0, 0, 0, 0];

			options.color[0] = parseInt(result[1], 16);
			options.color[1] = parseInt(result[2], 16);
			options.color[2] = parseInt(result[3], 16);
			options.color[3] = 200;
		}
    }
	else {
	    options.color = [255, 0, 0, 200];
	}

	this.polyline_elements_count = 0;
	this.sprite_elements_count = 0;
	this.frame_number = 0;
	this.line_pattern = [1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0];

	var vertex_count = options.count;

    if (vertex_count < 2)
    	return;

	var segments = [];

	var x1 = 6378137 * options.lng[0] * (Math.PI/180);
	var y1 = 6378137 * Math.log(Math.tan(Math.PI/4+options.lat[0]*(Math.PI/180)/2)); 

	var lat = options.lat;
	var lng = options.lng;

	for (var v = 1; v < vertex_count; v++) {

		var x2 = 6378137 * lng[v] * (Math.PI/180);
		var y2 = 6378137 * Math.log(Math.tan(Math.PI/4+lat[v]*(Math.PI/180)/2)); 

		var dx = x2 - x1;
		var dy = y2 - y1;

		var vector_len = Math.sqrt((dx * dx) + (dy * dy));

		dx = dx / vector_len;
		dy = dy / vector_len;

		var x45 = 0.7071067811865475244 * (dx - dy);
		var y45 = 0.7071067811865475244 * (dx + dy);

		var segment = {
			x1: x1,
			y1: y1,
			x2: x2,
			y2: y2,

			normal_px: -dy,
			normal_py: dx,
			normal_nx: dy,
			normal_ny: -dx,

			normal_ax: x45,
			normal_ay: y45,			
			normal_bx: -y45,
			normal_by: x45,				
			normal_cx: -x45,
			normal_cy: -y45,				
			normal_dx: y45,
			normal_dy: -x45,
		};

		segments.push(segment);

		x1 = x2;
		y1 = y2;
	}

	// Количество сегментов полилинии
	var segments_count = segments.length;

	// Каждый сегмент рисуется как прямоугольник состоящий из двух треугольников
	// Каждому сегменту нужно:
	// - четыре вершинных координаты (x,y), по факту передаются две одинаковые, правятся в вершинном шейдере в направлении нормалей
	// - четыре нормали (x0, y0)
	// - четыре текстурных координаты (s, t)
	// - шесть индексов (ushort)
	
	var position_data = new Float32Array(segments_count * 4 * 2);
	var normal_data   = new Float32Array(segments_count * 4 * 2);
	var texcoord_data = new Float32Array(segments_count * 4 * 2);
	var index_data    = new Uint16Array(segments_count * 6);

	var position_pointer = 0;
	var normal_pointer   = 0;
	var texcoord_pointer = 0;
	var index_pointer    = 0;

	for (var s = 0; s < segments_count; s++) {

		var segment = segments[s];

		var x1 = segment.x1;
		var y1 = segment.y1;

		var x2 = segment.x2;
		var y2 = segment.y2;

		position_data[position_pointer++] = x1;
		position_data[position_pointer++] = y1;
		position_data[position_pointer++] = x1;
		position_data[position_pointer++] = y1;

		position_data[position_pointer++] = x2;
		position_data[position_pointer++] = y2;
		position_data[position_pointer++] = x2;
		position_data[position_pointer++] = y2;

		var normal_px = segment.normal_px;
		var normal_py = segment.normal_py;

		var normal_nx = segment.normal_nx;
		var normal_ny = segment.normal_ny;

		normal_data[normal_pointer++] = normal_px;
		normal_data[normal_pointer++] = normal_py;
		normal_data[normal_pointer++] = normal_nx;
		normal_data[normal_pointer++] = normal_ny;

		normal_data[normal_pointer++] = normal_px;
		normal_data[normal_pointer++] = normal_py;
		normal_data[normal_pointer++] = normal_nx;
		normal_data[normal_pointer++] = normal_ny;

		texcoord_data[texcoord_pointer++] = 0;
		texcoord_data[texcoord_pointer++] = 0;
		texcoord_data[texcoord_pointer++] = 1;
		texcoord_data[texcoord_pointer++] = 0;
		texcoord_data[texcoord_pointer++] = 0;
		texcoord_data[texcoord_pointer++] = 1;
		texcoord_data[texcoord_pointer++] = 1;
		texcoord_data[texcoord_pointer++] = 1;

		var base_point = s * 4;

		index_data[index_pointer++] = base_point + 0;
		index_data[index_pointer++] = base_point + 1;
		index_data[index_pointer++] = base_point + 2;
		index_data[index_pointer++] = base_point + 1;
		index_data[index_pointer++] = base_point + 2;
		index_data[index_pointer++] = base_point + 3;
	}

	this.polyline_position_data  = position_data;
	this.polyline_normal_data    = normal_data;
	this.polyline_texcoord_data  = texcoord_data;
	this.polyline_index_data     = index_data;
	if (segments_count > 10922)
		segments_count = 10922;
	this.polyline_elements_count = segments_count * 6;

	// Количество спрайтов
	var sprites_count = segments.length + 1;

	// Каждый спрайт рисуется как квадрат состоящий из двух треугольников
	// Каждому спрайту нужно:
	// - четыре вершинных координаты (x,y), по факту передаются все одинаковые, правятся в вершинном шейдере в направлении нормалей
	// - четыре нормали (x0, y0)
	// - четыре текстурных координаты (s, t)
	// - шесть индексов (ushort)
	
	position_data = new Float32Array(sprites_count * 4 * 2);
	normal_data   = new Float32Array(sprites_count * 4 * 2);
	texcoord_data = new Float32Array(sprites_count * 4 * 2);
	index_data    = new Uint16Array(sprites_count * 6);

	position_pointer = 0;
	normal_pointer   = 0;
	texcoord_pointer = 0;
	index_pointer    = 0;

	for (var s = 0; s < segments_count; s++) {

		var segment = segments[s];

		var x1 = segment.x1;
		var y1 = segment.y1;

		var x2 = segment.x2;
		var y2 = segment.y2;

		position_data[position_pointer++] = x1;
		position_data[position_pointer++] = y1;
		position_data[position_pointer++] = x1;
		position_data[position_pointer++] = y1;
		position_data[position_pointer++] = x1;
		position_data[position_pointer++] = y1;
		position_data[position_pointer++] = x1;
		position_data[position_pointer++] = y1;

		normal_data[normal_pointer++] = segment.normal_ax;
		normal_data[normal_pointer++] = segment.normal_ay;
		normal_data[normal_pointer++] = segment.normal_bx;
		normal_data[normal_pointer++] = segment.normal_by;
		normal_data[normal_pointer++] = segment.normal_cx;
		normal_data[normal_pointer++] = segment.normal_cy;
		normal_data[normal_pointer++] = segment.normal_dx;
		normal_data[normal_pointer++] = segment.normal_dy;

		texcoord_data[texcoord_pointer++] = 0;
		texcoord_data[texcoord_pointer++] = 1;
		texcoord_data[texcoord_pointer++] = 0;
		texcoord_data[texcoord_pointer++] = 0;
		texcoord_data[texcoord_pointer++] = 1;
		texcoord_data[texcoord_pointer++] = 0;
		texcoord_data[texcoord_pointer++] = 1;
		texcoord_data[texcoord_pointer++] = 1;

		var base_point = s * 4;

		index_data[index_pointer++] = base_point + 0;
		index_data[index_pointer++] = base_point + 1;
		index_data[index_pointer++] = base_point + 2;
		index_data[index_pointer++] = base_point + 0;
		index_data[index_pointer++] = base_point + 2;
		index_data[index_pointer++] = base_point + 3;
	}

	var segment = segments[segments_count - 1];

	var x1 = segment.x2;
	var y1 = segment.y2;

	position_data[position_pointer++] = x1;
	position_data[position_pointer++] = y1;
	position_data[position_pointer++] = x1;
	position_data[position_pointer++] = y1;
	position_data[position_pointer++] = x1;
	position_data[position_pointer++] = y1;
	position_data[position_pointer++] = x1;
	position_data[position_pointer++] = y1;

	normal_data[normal_pointer++] = segment.normal_ax;
	normal_data[normal_pointer++] = segment.normal_ay;
	normal_data[normal_pointer++] = segment.normal_bx;
	normal_data[normal_pointer++] = segment.normal_by;
	normal_data[normal_pointer++] = segment.normal_cx;
	normal_data[normal_pointer++] = segment.normal_cy;
	normal_data[normal_pointer++] = segment.normal_dx;
	normal_data[normal_pointer++] = segment.normal_dy;

	texcoord_data[texcoord_pointer++] = 0;
	texcoord_data[texcoord_pointer++] = 1;
	texcoord_data[texcoord_pointer++] = 0;
	texcoord_data[texcoord_pointer++] = 0;
	texcoord_data[texcoord_pointer++] = 1;
	texcoord_data[texcoord_pointer++] = 0;
	texcoord_data[texcoord_pointer++] = 1;
	texcoord_data[texcoord_pointer++] = 1;

	var base_point = segments_count * 4;

	index_data[index_pointer++] = base_point + 0;
	index_data[index_pointer++] = base_point + 1;
	index_data[index_pointer++] = base_point + 2;
	index_data[index_pointer++] = base_point + 0;
	index_data[index_pointer++] = base_point + 2;
	index_data[index_pointer++] = base_point + 3;

	this.sprite_position_data  = position_data;
	this.sprite_normal_data    = normal_data;
	this.sprite_texcoord_data  = texcoord_data;
	this.sprite_index_data     = index_data;
	if (sprites_count > 10922)
		sprites_count = 10922;
	this.sprite_elements_count = sprites_count * 6;

	this.updateSpriteData();
	this.updateLinePatternData();
};

PolylineGL.prototype.restore = function(gl) {

	this.polyline_shader = this.options.layer.renderer.polyline_shader;

	this.gl = gl;

	// POLYLINE POSITIONS
	var polyline_position_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, polyline_position_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.polyline_position_data, gl.STATIC_DRAW);

	this.polyline_position_buffer = polyline_position_buffer;

	// POLYLINE NORMAL
	var polyline_normal_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, polyline_normal_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.polyline_normal_data, gl.STATIC_DRAW);

	this.polyline_normal_buffer = polyline_normal_buffer;

	// POLYLINE TEXTURE COORDS
	var polyline_texcoord_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, polyline_texcoord_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.polyline_texcoord_data, gl.STATIC_DRAW);

	gl.bindBuffer(gl.ARRAY_BUFFER, null);

	this.polyline_texcoord_buffer = polyline_texcoord_buffer;

	// POLYLINE INDEX
	var polyline_index_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, polyline_index_buffer);

	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.polyline_index_data, gl.STATIC_DRAW);

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);

	this.polyline_index_buffer = polyline_index_buffer;

	// SPRITE POSITIONS
	var sprite_position_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, sprite_position_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.sprite_position_data, gl.STATIC_DRAW);

	this.sprite_position_buffer = sprite_position_buffer;

	// SPRITE NORMAL
	var sprite_normal_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, sprite_normal_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.sprite_normal_data, gl.STATIC_DRAW);

	this.sprite_normal_buffer = sprite_normal_buffer;

	// SPRITE TEXTURE COORDS
	var sprite_texcoord_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ARRAY_BUFFER, sprite_texcoord_buffer);

	gl.bufferData(gl.ARRAY_BUFFER, this.sprite_texcoord_data, gl.STATIC_DRAW);

	this.sprite_texcoord_buffer = sprite_texcoord_buffer;

	// SPRITE INDEX
	var sprite_index_buffer = gl.createBuffer();

	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, sprite_index_buffer);

	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.sprite_index_data, gl.STATIC_DRAW);

	this.sprite_index_buffer = sprite_index_buffer;

	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);

	this.updateSpriteTexture();
	this.updateLinePatternTexture();
};

PolylineGL.prototype.lost = function() {
	this.gl = null;
};

PolylineGL.prototype.setColor = function(color) {

	this.color = color;
	
	updateSpriteData();
	updateSpriteTexture();
	updateLinePatternData();
	updateLinePatternTexture();

	return this;
};

PolylineGL.prototype.setLinePattern = function(line_pattern) {

	this.line_pattern = line_pattern;
	
	updateLinePatternData();
	updateLinePatternTexture();

	return this;
};

PolylineGL.prototype.updateLinePatternData = function() {

	var color = this.options.color;
	var line_pattern = this.line_pattern;

	var polyline_texture_data = new Uint8Array(line_pattern.length * 4);

	for (var i = 0; i < line_pattern.length; i++) {
	
		if (line_pattern[i] == 0) {
			polyline_texture_data[i * 4 + 3] = 0;
		}
		else {
			var offset = i * 4;
			polyline_texture_data[offset + 0] = color[0];
			polyline_texture_data[offset + 1] = color[1];
			polyline_texture_data[offset + 2] = color[2];
			polyline_texture_data[offset + 3] = color[3];
		}
	}

	this.polyline_texture_data = polyline_texture_data;
};

PolylineGL.prototype.updateLinePatternTexture = function() {

	var gl = this.gl;

	if (gl === null)
		return;

	var polyline_texture_data = this.polyline_texture_data;

	var polyline_texture = this.polyline_texture;

	if (polyline_texture == null) {
		polyline_texture = gl.createTexture();
		this.polyline_texture = polyline_texture;
	}

	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, polyline_texture);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, this.line_pattern.length, 0, gl.RGBA, gl.UNSIGNED_BYTE, polyline_texture_data);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);

	gl.bindTexture(gl.TEXTURE_2D, null);
};

PolylineGL.prototype.updateSpriteData = function() {

	var color = this.options.color;
	var sprite_texture_data = this.sprite_texture_data;

	var source = [0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0];
	
	if (sprite_texture_data == null) {
		var sprite_texture_data = new Uint8Array(16 * 16 * 4);
		this.sprite_texture_data = sprite_texture_data;
	}

	for (var i = 0; i < source.length; i++) {
	
		if (source[i] == 0) {
			sprite_texture_data[i * 4 + 3] = 0;
		}
		else {
			var offset = i * 4;
			sprite_texture_data[offset + 0] = color[0];
			sprite_texture_data[offset + 1] = color[1];
			sprite_texture_data[offset + 2] = color[2];
			sprite_texture_data[offset + 3] = 255;
		}
	}
};

PolylineGL.prototype.updateSpriteTexture = function() {

	var gl = this.gl;

	if (gl === null)
		return;

	var sprite_texture_data = this.sprite_texture_data;

	var sprite_texture = this.sprite_texture;

	if (sprite_texture == null) {
		sprite_texture = gl.createTexture();
		this.sprite_texture = sprite_texture;
	}

	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, sprite_texture);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 16, 16, 0, gl.RGBA, gl.UNSIGNED_BYTE, sprite_texture_data);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);

	gl.bindTexture(gl.TEXTURE_2D, null);
};

PolylineGL.prototype.cleanup = function() {
	
	var gl = this.gl;

	if (gl === null)
		return;

    var polyline_position_buffer = this.polyline_position_buffer;    	
	gl.deleteBuffer(polyline_position_buffer);
    
    var polyline_normal_buffer = this.polyline_normal_buffer;    	
	gl.deleteBuffer(polyline_normal_buffer);

    var polyline_texcoord_buffer = this.polyline_texcoord_buffer;    	
	gl.deleteBuffer(polyline_texcoord_buffer);
    
    var polyline_index_buffer = this.polyline_index_buffer;    	
	gl.deleteBuffer(polyline_index_buffer);

	var polyline_texture = this.polyline_texture;
	gl.deleteTexture(polyline_texture);

    var sprite_position_buffer = this.sprite_position_buffer;    	
	gl.deleteBuffer(sprite_position_buffer);
    
    var sprite_normal_buffer = this.sprite_normal_buffer;    	
	gl.deleteBuffer(sprite_normal_buffer);

    var sprite_texcoord_buffer = this.sprite_texcoord_buffer;    	
	gl.deleteBuffer(sprite_texcoord_buffer);
    
    var sprite_index_buffer = this.sprite_index_buffer;    	
	gl.deleteBuffer(sprite_index_buffer);

	var sprite_texture = this.sprite_texture;
	gl.deleteTexture(sprite_texture);
};

PolylineGL.prototype.render = function(proj, modelView) {

	var gl							= this.gl;
	var polyline_shader 			= this.polyline_shader;
	
	var polylineDrawShaderProgram	= polyline_shader.polylineDrawShaderProgram;
	var spriteDrawShaderProgram		= polyline_shader.spriteDrawShaderProgram;

	gl.activeTexture(gl.TEXTURE0);

	/****************************************/
	/* PolylineGL                           */
	/****************************************/

	gl.useProgram(polylineDrawShaderProgram);

    /* Uniforms */
	gl.uniform1f(polylineDrawShaderProgram.frameLengthUniform, this.line_pattern.length); 
	gl.uniform1f(polylineDrawShaderProgram.lineWidthUniform, this.options.width / 2); 
	gl.uniform1f(polylineDrawShaderProgram.frameNumberUniform, this.frame_number++); 
	gl.uniform1i(polylineDrawShaderProgram.samplerUniform, 0);
	gl.uniformMatrix4fv(polylineDrawShaderProgram.modelviewMatrixUniform, false, modelView); 
	gl.uniformMatrix4fv(polylineDrawShaderProgram.projMatrixUniform, false, proj);

	/* Attributes */
	gl.enableVertexAttribArray(polylineDrawShaderProgram.posAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.polyline_position_buffer);
    gl.vertexAttribPointer(polylineDrawShaderProgram.posAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.enableVertexAttribArray(polylineDrawShaderProgram.normalAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.polyline_normal_buffer);
    gl.vertexAttribPointer(polylineDrawShaderProgram.normalAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.enableVertexAttribArray(polylineDrawShaderProgram.textureAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.polyline_texcoord_buffer);
    gl.vertexAttribPointer(polylineDrawShaderProgram.textureAttrib, 2, gl.FLOAT, false, 0, 0);

	/* Texture */
	gl.bindTexture(gl.TEXTURE_2D, this.polyline_texture);

	/* Indices */
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.polyline_index_buffer);

	/* Draw */
	gl.drawElements(gl.TRIANGLES, this.polyline_elements_count, gl.UNSIGNED_SHORT, 0);

	gl.disableVertexAttribArray(polylineDrawShaderProgram.posAttrib);
	gl.disableVertexAttribArray(polylineDrawShaderProgram.normalAttrib);
	gl.disableVertexAttribArray(polylineDrawShaderProgram.textureAttrib);

	/****************************************/
	/* Sprites                              */
	/****************************************/

	gl.useProgram(spriteDrawShaderProgram);

    /* Uniforms */
	gl.uniform1i(spriteDrawShaderProgram.samplerUniform, 0);
	gl.uniformMatrix4fv(spriteDrawShaderProgram.modelviewMatrixUniform, false, modelView); 
	gl.uniformMatrix4fv(spriteDrawShaderProgram.projMatrixUniform, false, proj);
	gl.uniform1f(spriteDrawShaderProgram.pointSizeUniform, Math.sqrt(2 * 32)); 

	/* Attributes */
	gl.enableVertexAttribArray(spriteDrawShaderProgram.posAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.sprite_position_buffer);
    gl.vertexAttribPointer(spriteDrawShaderProgram.posAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.enableVertexAttribArray(spriteDrawShaderProgram.normalAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.sprite_normal_buffer);
    gl.vertexAttribPointer(spriteDrawShaderProgram.normalAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.enableVertexAttribArray(spriteDrawShaderProgram.textureAttrib);    
	gl.bindBuffer(gl.ARRAY_BUFFER, this.sprite_texcoord_buffer);
    gl.vertexAttribPointer(spriteDrawShaderProgram.textureAttrib, 2, gl.FLOAT, false, 0, 0);

	/* Texture */
	gl.bindTexture(gl.TEXTURE_2D, this.sprite_texture);

	/* Indices */
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.sprite_index_buffer);

	/* Draw */
	gl.drawElements(gl.TRIANGLES, this.sprite_elements_count, gl.UNSIGNED_SHORT, 0); 

	gl.disableVertexAttribArray(spriteDrawShaderProgram.posAttrib);
	gl.disableVertexAttribArray(spriteDrawShaderProgram.normalAttrib);
	gl.disableVertexAttribArray(spriteDrawShaderProgram.textureAttrib);    

	gl.bindTexture(gl.TEXTURE_2D, null);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);   
};

PolylineGL.prototype.lookup = function(proj, modelView, id_buffer) {

	var gl						= this.gl;
	var polyline_shader 		= this.polyline_shader;

	var polylineShaderProgram	= polyline_shader.polylineSelectShaderProgram;
	var spriteShaderProgram		= polyline_shader.spriteSelectShaderProgram;

	gl.bindTexture(gl.TEXTURE_2D, null);

	/****************************************/
	/* PolylineGL                             */
	/****************************************/

	gl.useProgram(polylineShaderProgram);

    /* Uniforms */
	gl.uniform1f(polylineShaderProgram.lineWidthUniform, this.options.width / 2); 
	gl.uniformMatrix4fv(polylineShaderProgram.modelviewMatrixUniform, false, modelView); 
	gl.uniformMatrix4fv(polylineShaderProgram.projMatrixUniform, false, proj);

	/* Attributes */
	gl.enableVertexAttribArray(polylineShaderProgram.posAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.polyline_position_buffer);
    gl.vertexAttribPointer(polylineShaderProgram.posAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.enableVertexAttribArray(polylineShaderProgram.normalAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.polyline_normal_buffer);
    gl.vertexAttribPointer(polylineShaderProgram.normalAttrib, 2, gl.FLOAT, false, 0, 0);

	/* Indices */
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.polyline_index_buffer);

	/* Draw */
	gl.drawElements(gl.TRIANGLES, this.polyline_elements_count, gl.UNSIGNED_SHORT, 0);

	gl.disableVertexAttribArray(polylineShaderProgram.posAttrib);
	gl.disableVertexAttribArray(polylineShaderProgram.normalAttrib);

	/****************************************/
	/* Sprites                              */
	/****************************************/

	gl.useProgram(spriteShaderProgram);

    /* Uniforms */
	gl.uniformMatrix4fv(spriteShaderProgram.modelviewMatrixUniform, false, modelView); 
	gl.uniformMatrix4fv(spriteShaderProgram.projMatrixUniform, false, proj);
	gl.uniform1f(spriteShaderProgram.pointSizeUniform, Math.sqrt(2 * 32)); 

	/* Attributes */
	gl.enableVertexAttribArray(spriteShaderProgram.posAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.sprite_position_buffer);
    gl.vertexAttribPointer(spriteShaderProgram.posAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.enableVertexAttribArray(spriteShaderProgram.normalAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, this.sprite_normal_buffer);
    gl.vertexAttribPointer(spriteShaderProgram.normalAttrib, 2, gl.FLOAT, false, 0, 0);

	gl.enableVertexAttribArray(spriteShaderProgram.idAttrib);
	gl.bindBuffer(gl.ARRAY_BUFFER, id_buffer);
    gl.vertexAttribPointer(spriteShaderProgram.idAttrib, 2, gl.FLOAT, false, 0, 0);

	/* Indices */
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.sprite_index_buffer);

	/* Draw */
	gl.drawElements(gl.TRIANGLES, this.sprite_elements_count, gl.UNSIGNED_SHORT, 0); 

	gl.disableVertexAttribArray(spriteShaderProgram.posAttrib);
	gl.disableVertexAttribArray(spriteShaderProgram.normalAttrib);
	gl.disableVertexAttribArray(spriteShaderProgram.idAttrib);

	gl.bindTexture(gl.TEXTURE_2D, null);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);   
};

PolylineGL.prototype.getPopup = function(vertex_index) {

	if (vertex_index >= 65535)
		return this.options.line_popup_text;

	if (typeof this.options.sprite_popup_text == 'string')
		return this.options.sprite_popup_text;

	if (typeof this.options.sprite_popup_text == 'function')
		return this.options.sprite_popup_text(Math.floor(vertex_index / 4));

	return this.options.line_popup_text;
};