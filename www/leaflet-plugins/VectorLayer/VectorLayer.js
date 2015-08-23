//******************************************************************************
//
// File Name : VectorLayer.js
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

L.TileLayer.VectorLayer = L.Class.extend({

	overlays: [],

    options: {
        size: 30000, // in meters
        opacity: 1,
		gradientTexture: false,
		alphaRange: 1,
		autoresize: false
    },
    
    initialize: function (options) {

        L.Util.setOptions(this, options);

		this.proj		= mat4.create();
		this.modelView	= mat4.create();

        this.last_render_time = 0;
    },

    onAdd: function (map) {

        this.map				= map;

		var options				= this.options;
		var mapsize				= map.getSize();
		
		var canvas				= document.createElement("canvas");
		canvas.id				= 'skytrack-leaflet';
        canvas.width			= mapsize.x;
        canvas.height			= mapsize.y;
        canvas.style.opacity	= options.opacity;
        canvas.style.position	= 'absolute';
		
		map.getPanes().overlayPane.appendChild(canvas);
        
		map.on("moveend",	this.onMove, this);
		map.on("zoomstart", this._hide, this);
		map.on("zoomend",	this.onZoom, this);
		map.on('resize',	this.onSize, this);
		map.on('mousemove', this.onMouseMove, this);

        this.canvas		= canvas;
		this.zoom		= map.getZoom();
		
		this.gl = this.getWebGL();

//		this.gl = null;

		if (this.gl !== null) {
			this.renderer = new WebGLRenderer(this, this.gl);
		}
		else {
			this.renderer = new CanvasRenderer(this);
		}
		         
		this.createPolyline = this.renderer.createPolyline;
		this.createMarkersArray = this.renderer.createMarkersArray;
		this.createTooltipsArray = this.renderer.createTooltipsArray;

		this.updateModelViewMatrix();
		
		window.requestAnimationFrm = (function(){
			return window.requestAnimationFrame    || 
				window.webkitRequestAnimationFrame || 
				window.mozRequestAnimationFrame    || 
				window.oRequestAnimationFrame      || 
				window.msRequestAnimationFrame     || 
				function(/* function */ callback, /* DOMElement */ element)
				{
					window.setTimeout(callback, 1000 / 60);
				};
    	})();

		this.animate();
	},

	getWebGL: function() {

		//return null;

		var gl = this.canvas.getContext("webgl", { antialias: false }) || this.canvas.getContext("experimental-webgl", { antialias: false });

		return gl;
	},

	updateModelViewMatrix: function() {

		var map			= this.map;
		var canvas		= this.canvas;
		var modelView	= this.modelView;
		var point		= map.latLngToContainerPoint([0, 0]);

		mat4.identity(modelView);

		var translate_x = point.x;
		var translate_y = canvas.height - point.y;
		mat4.translate(modelView, [translate_x, translate_y, 0]);

		var scale = Math.pow(2, 8 + this.zoom) / 40075000;
		mat4.scale(modelView, [scale, scale, 0]);

		this.scale = scale;
		this.translate_x = point.x;
		this.translate_y = point.y;
	},

	animate: function(time) {

		var that = this;		
		window.requestAnimationFrm(function(time) { that.animate(time); });

		if (time > this.last_render_time + 100) {
			this.last_render_time = time;
			this.frameNumber++;
			this.renderer.render();
		}
	},

	onRemove: function (map) {

		var overlays = this.overlays;

        map.getPanes().overlayPane.removeChild(this.canvas);

		map.off("moveend",		this.onMove,		this);
		map.off("zoomstart",	this._hide,			this);
		map.off("zoomend",		this.onZoom,		this);
		map.off('resize',		this.onSize,		this);
		map.off('mousemove',	this.onMouseMove,	this);

		this.renderer.cleanup();
    },

	onZoom : function () {
		this.zoom = this.map.getZoom();
		this._show();
		this.updateModelViewMatrix();
		this.renderer.render();
	},
	
	onMove: function() {
	    
	    var map = this.map;

		L.DomUtil.setPosition(this.canvas, map.latLngToLayerPoint(map.getBounds().getNorthWest()));

		this.updateModelViewMatrix();
		this.renderer.render();
	},

	onSize: function (event) {

		var canvas	= this.canvas;
		var x		= event.newSize.x;
		var y		= event.newSize.y;

		canvas.width	= x;
		canvas.height	= y;

		this.updateModelViewMatrix();
		this.renderer.resize(x, y);
	},	

	onMouseMove: function(event) {

		var self	= this;
		var canvas	= self.canvas;

		if ((this.mouse_x == event.containerPoint.x)&&(this.mouse_y == event.containerPoint.y))
			return;

		this.mouse_x = event.containerPoint.x;
		this.mouse_y = event.containerPoint.y;

		window.clearTimeout(this.timeout_id);

		this.timeout_id = window.setTimeout( 

			function() { 
	
				window.clearTimeout(self.timeout_id);

				var x = event.containerPoint.x;
				var y = canvas.height - event.containerPoint.y;

				var object = self.renderer.lookup(x, y);

				if (typeof object == 'string') {
					self.popup = L.popup()
					   .setLatLng(event.latlng) 
					   .setContent(object)
					   .openOn(self.map);
				}
				else {
					self.map.closePopup();
				}
			}, 

			250
		);
	},

	_hide : function () {
		this.map.closePopup();
		this.canvas.style.display = 'none';
	},
	
	_show : function () {
		this.canvas.style.display = 'block';
	},

	addOverlay: function(o) {
		return this.renderer.addOverlay(o);
	},

	removeOverlay: function(o) {
		return this.renderer.removeOverlay(o);
	}

});

L.TileLayer.vectorlayer = function (options) {
    return new L.TileLayer.SkytrackLayer(options);
};