//******************************************************************************
//
// File Name : Renderer.js
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

function WebGLRenderer(layer, gl) 
{
	this.layer		= layer;
	this.gl			= gl;
	this.proj		= layer.proj;
	this.modelView	= layer.modelView;

	this.polyline_shader = new PolylineShader({});

	this.overlays = [];

	this.restore(gl);
}

WebGLRenderer.prototype.restore = function(gl) 
{

	gl.clearColor(0,0,0,0);
	gl.clearStencil(0); 
	gl.disable(gl.DEPTH_TEST);
	gl.enable(gl.BLEND);
	gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	     		
	var id_data = new Float32Array(65536);
	
	for (var i = 0; i < 32768; i++) {
		var lo = i & 0xFF;
		var hi = (i >> 8) & 0xFF;
		id_data[i * 2 + 0] = lo / 255;
		id_data[i * 2 + 1] = hi / 255;
	}

	var id_buffer	= gl.createBuffer();
	this.id_buffer	= id_buffer;
	
	gl.bindBuffer(gl.ARRAY_BUFFER, id_buffer);
	gl.bufferData(gl.ARRAY_BUFFER, id_data, gl.STATIC_DRAW);
	gl.bindBuffer(gl.ARRAY_BUFFER, null);

	this.polyline_shader.restore(gl);

	var overlays = this.overlays;

	for (var i = overlays.length; i--;)		
		overlays[i].restore(gl);
};

WebGLRenderer.prototype.render = function() {

	var gl				= this.gl;
	var overlays		= this.overlays;
	var proj			= this.proj;
	var modelView		= this.modelView;
	var overlays_count	= overlays.length;

	gl.clear(gl.COLOR_BUFFER_BIT);

	for (var i = 0; i < overlays_count; i++)		
		overlays[i].render(proj, modelView);
};

WebGLRenderer.prototype.lookup = function(x, y) {

	var gl				= this.gl;
	var overlays		= this.overlays;
	var proj			= this.proj;
	var modelView		= this.modelView;
	var overlays_count	= overlays.length;

	var result;
	var id_buffer = this.id_buffer;

	var pixel = new Uint8Array(4);

	var tex = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, tex);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.layer.canvas.width, this.layer.canvas.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);

	var fb = gl.createFramebuffer();
	gl.bindFramebuffer(gl.FRAMEBUFFER, fb);
	gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, tex, 0);

	for (var i = 0; i < overlays_count; i++) {
	
		var overlay = overlays[i];

		gl.clear(gl.COLOR_BUFFER_BIT);

		overlay.lookup(proj, modelView, id_buffer);

		gl.readPixels(x, y, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, pixel);
			
		if ((pixel[0] != 0)||(pixel[1] != 0)||(pixel[2] != 0)||(pixel[3] != 0)) {	

			vertex_number = pixel[0] + pixel[1] * 256;

			result = overlay.getPopup(vertex_number);
		}
	}

	gl.bindFramebuffer(gl.FRAMEBUFFER, null);

	gl.deleteFramebuffer(fb);
	gl.deleteTexture(tex);

	return result;
};

WebGLRenderer.prototype.addOverlay = function(o) {

	this.overlays.push(o);

	o.restore(this.gl);

	function compare(a,b) {
	  if (a.options.z_index < b.options.z_index)
	     return -1;
	  if (a.options.z_index > b.options.z_index)
	    return 1;
	  return 0;
	}

	this.overlays.sort(compare);
};

WebGLRenderer.prototype.removeOverlay = function(o) {

	var overlays = this.overlays;

	var i;
	while ((i = overlays.indexOf(o)) != -1) {
		overlays.splice(i, 1);
	}

	if (typeof o == 'object')
		o.cleanup();
};

WebGLRenderer.prototype.cleanup = function() {

	var overlays = this.overlays;

	for (var i = overlays.length; i--;)		
		overlays[i].cleanup();

	this.overlays = [];

	this.polyline_shader.cleanup();
};

WebGLRenderer.prototype.resize = function (x, y) {

	this.gl.viewport(0, 0, x, y);
	mat4.ortho(0, x, 0, y, -1, 1, this.proj);
	
	this.render();
};

WebGLRenderer.prototype.createPolyline = function(options) {
	var polyline = new PolylineGL(options);
	return polyline;
};

WebGLRenderer.prototype.createMarkersArray = function(options) {
	var ma = new MarkerArrayGL(options);
	return ma;
};

WebGLRenderer.prototype.createTooltipsArray = function(options) {
	var ta = new TooltipsArrayGL(options);
	return ta;
};