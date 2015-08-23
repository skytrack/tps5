//******************************************************************************
//
// File Name : Polyline.js
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

function PolylineCanvas(options) {

	this.options = options;
    
    if (typeof options.color != 'undefined') {
		
		var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(options.color);

		if (result) {
			options.color = [0, 0, 0, 0];

			options.color[0] = parseInt(result[1], 16);
			options.color[1] = parseInt(result[2], 16);
			options.color[2] = parseInt(result[3], 16);
		}
    }
	else {
	    options.color = [255, 0, 0, 200];
	}

	this.frame_number = 0;
	this.line_pattern = [1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0];

    this.canvas		= this.options.layer.canvas;
    this.ctx		= this.options.layer.renderer.ctx;

    this.frame_number = 0;

    var vertex_count = options.count;

    if (vertex_count < 2)
    	return;

	var lat				= options.lat;
	var lng				= options.lng;
	var xy_data			= new Uint32Array(vertex_count * 2);
	var position_index	= 0;

	for (var i = 0; i < vertex_count; i++) {

		xy_data[position_index++] = 6378137 * lng[i] * (Math.PI/180);
		xy_data[position_index++] = 6378137 * Math.log(Math.tan(Math.PI/4+lat[i]*(Math.PI/180)/2)); 
	}

 	this.xy_data	= xy_data;
	this.xy_len		= position_index;

	this.updateSpriteCanvas();
	this.updateLinePatternData();
};

PolylineCanvas.prototype.setColor = function(color) {

	this.color = color;
	
	updateSpriteCanvas();
	updateLinePatternData();

	return this;
};

PolylineCanvas.prototype.setLinePattern = function(line_pattern) {

	this.line_pattern = line_pattern;
	
	updateLinePatternData();

	return this;
};

PolylineCanvas.prototype.updateLinePatternData = function() {

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

PolylineCanvas.prototype.updateSpriteCanvas = function() {

	var color = this.options.color;

	var sprite_canvas		= document.createElement("canvas");
	var sprite_ctx			= sprite_canvas.getContext("2d");

	sprite_canvas.width	 = 10;
	sprite_canvas.height = 10;

	sprite_ctx.beginPath();
	sprite_ctx.arc(5, 5, 5, 0, 2 * Math.PI, false);
	sprite_ctx.fillStyle = this.rgbToHex(color[0], color[1], color[2]);
	sprite_ctx.fill();
	sprite_ctx.lineWidth = 0;
	sprite_ctx.strokeStyle = this.rgbToHex(color[0], color[1], color[2]);
	sprite_ctx.stroke();

	this.sprite_canvas = sprite_canvas;
	this.sprite_ctx = sprite_ctx;
};

PolylineCanvas.prototype.rgbToHex = function(r, g, b) {

	function componentToHex(c) {
    	var hex = c.toString(16);
    	return hex.length == 1 ? "0" + hex : hex;
	}
    
    return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}

PolylineCanvas.prototype.render = function() 
{
	var ctx = this.ctx;
	var xy	= this.xy_data;
	var len	= this.xy_len;
	var color = this.options.color;
	
	var scale = this.options.layer.scale;
	var translate_x = this.options.layer.translate_x;
	var translate_y = this.options.layer.translate_y;

	var frame_number = this.frame_number;
	frame_number++;
	this.frame_number = frame_number;

	ctx.lineWidth 		= 4;
	ctx.strokeStyle 	= this.rgbToHex(color[0], color[1], color[2]);
	ctx.lineJoin		= 'bevel';
	ctx.lineDashOffset = -frame_number;	

	if ( ctx.setLineDash !== undefined )   ctx.setLineDash([10]);
	if ( ctx.mozDash !== undefined )       ctx.mozDash = [10];

	if (len > 2) {

		ctx.beginPath();

		ctx.moveTo(xy[0] * scale + translate_x, translate_y - xy[1] * scale);

		for (var i = 2; i < len; i += 2) {
			ctx.lineTo(xy[i] * scale + translate_x, translate_y - xy[i + 1] * scale);
		}

		ctx.stroke();
	}

	var sprite = this.sprite_canvas;
	for (var i = 0; i < len; i += 2) {
		ctx.drawImage(sprite, xy[i] * scale + translate_x - 5, translate_y - xy[i + 1] * scale - 5);
	}
};

PolylineCanvas.prototype.lookup = function(x, y) 
{
	var ctx = this.ctx;
	var xy	= this.xy_data;
	var len	= this.xy_len;
	var color = this.options.color;
	
	var scale = this.options.layer.scale;
	var translate_x = this.options.layer.translate_x;
	var translate_y = this.options.layer.translate_y;

	ctx.lineWidth 		= 4;
	ctx.strokeStyle 	= this.rgbToHex(color[0], color[1], color[2]);
	ctx.lineJoin		= 'bevel';

	var result;

	if (len > 2) {

		var image_data = ctx.getImageData(x, y, 1, 1);
		image_data.data[0] = 0;
		image_data.data[1] = 0;
		image_data.data[2] = 0;
		image_data.data[3] = 255;

		ctx.beginPath();

		ctx.moveTo(xy[0] * scale + translate_x, translate_y - xy[1] * scale);

		for (var i = 2; i < len; i += 2) {
			ctx.lineTo(xy[i] * scale + translate_x, translate_y - xy[i + 1] * scale);
		}

		ctx.stroke();

		var image_data = ctx.getImageData(x, y, 1, 1).data;

		if ((image_data[0] != 0)||(image_data[1] != 0)||(image_data[2] != 0))
			result = 0;
	}

	var sprite = this.sprite_canvas;
	var sprite_ctx = this.sprite_ctx;

	for (var i = 0; i < len; i += 2) {
		
		var sx = xy[i] * scale + translate_x - 5;
		var sy = translate_y - xy[i + 1] * scale - 5;

		if ((x >= sx)&&(x <= sx + 10)&&(y >= sy)&&(y <= sy + 10)) {

			var image_data = ctx.getImageData(x, y, 1, 1);
	
			image_data.data[0] = 0;
			image_data.data[1] = 0;
			image_data.data[2] = 0;
			image_data.data[3] = 255;
		
			ctx.putImageData(image_data, x, y);

			ctx.drawImage(sprite, sx, sy);

			var image_data = ctx.getImageData(x, y, 1, 1).data;

			if ((image_data[0] != 0)||(image_data[1] != 0)||(image_data[2] != 0))
				result = (i / 2) + 1;
		}
	}

	return result;
};

PolylineCanvas.prototype.getPopup = function(vertex_index) 
{
	if (vertex_index == 0)
		return this.options.line_popup_text;

	if (typeof this.options.sprite_popup_text == 'string')
		return this.options.sprite_popup_text;

	if (typeof this.options.sprite_popup_text == 'function')
		return this.options.sprite_popup_text(vertex_index - 1);
};