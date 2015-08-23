//******************************************************************************
//
// File Name : Renderer.js
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

function CanvasRenderer(layer) 
{
	this.layer		= layer;
	this.canvas		= layer.canvas;
	this.ctx		= this.canvas.getContext('2d');
	this.overlays	= [];
}

CanvasRenderer.prototype.render = function() 
{
	var overlays		= this.overlays;
	var overlays_count	= overlays.length;
	var ctx				= this.ctx;
	var layer			= this.layer;

	ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

	for (var i = 0; i < overlays_count; i++)		
		overlays[i].render();
};

CanvasRenderer.prototype.lookup = function(x, y) 
{
	var overlays		= this.overlays;
	var overlays_count	= overlays.length;
	var ctx				= this.ctx;
	var layer			= this.layer;

	var result;

	y = this.canvas.height - y;

	for (var i = 0; i < overlays_count; i++) {		
		ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
		
		var index = overlays[i].lookup(x, y);

		if (typeof index != 'undefined')
			result = overlays[i].getPopup(index);
	}

	this.render();

	return result;
};

CanvasRenderer.prototype.addOverlay = function(o) 
{
	this.overlays.push(o);

	function compare(a,b) {
	  if (a.options.z_index < b.options.z_index)
	     return -1;
	  if (a.options.z_index > b.options.z_index)
	    return 1;
	  return 0;
	}

	this.overlays.sort(compare);
};

CanvasRenderer.prototype.removeOverlay = function(o) 
{
	var overlays = this.overlays;

	var i;
	while ((i = overlays.indexOf(o)) != -1) {
		overlays.splice(i, 1);
	}
};

CanvasRenderer.prototype.cleanup = function() 
{
	this.overlays = [];
};

CanvasRenderer.prototype.resize = function (x, y) 
{
	this.render();
};

CanvasRenderer.prototype.createPolyline = function(options) 
{
	return new PolylineCanvas(options);
};

CanvasRenderer.prototype.createMarkersArray = function(options) {
	return new MarkerArrayCanvas(options);
};

CanvasRenderer.prototype.createTooltipsArray = function(options) {
	return new TooltipsArrayCanvas(options);
};