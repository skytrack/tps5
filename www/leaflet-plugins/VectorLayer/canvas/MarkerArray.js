function MarkerArrayCanvas(options) 
{
	var default_options = {
		url: '',
		width: 1,
		height: 1,
		sprites_count: 0,
		offset_x: 0,
		offset_y: 0,
		layer: null
	};

   	for (var key in options) {
   		default_options[key] = options[key];
   	}

    this.sprite_number	= 0;
    this.elements_count	= 0;
    this.markers		= [];
   	this.options		= default_options;

	var image			= new Image();

    image.loaded		= false;
	image.owner			= this;
	image.onload		= this.onImageLoad;
	image.src			= this.options.url;

    this.image			= image;
    this.canvas			= this.options.layer.canvas;
    this.ctx			= this.options.layer.renderer.ctx;

    this.xy_data		= [];
    this.xy_len			= 0;
};

MarkerArrayCanvas.prototype.onImageLoad = function() 
{ 
	var self = this.owner;

	this.loaded = true;
};

MarkerArrayCanvas.prototype.addMarker = function(marker) 
{
	var markers = this.markers;
	markers.push(marker);

	if (this.updating != true)
		this.endUpdate();

	return this;
};

MarkerArrayCanvas.prototype.removeMarker = function(marker) 
{
	var markers = this.markers;

	var i;
	while ((i = markers.indexOf(marker)) != -1) {
		markers.splice(i, 1);
	}

	if (this.updating !== true)
		this.endUpdate();

	return this;
};

MarkerArrayCanvas.prototype.removeAll = function() 
{
	this.markers	= [];
	this.xy_len		= 0;

	return this;
};

MarkerArrayCanvas.prototype.render = function() 
{
	var image = this.image;

	if (image.loaded) {

		var ctx = this.ctx;
		var xy	= this.xy_data;
		var len	= this.xy_len;

		var scale = this.options.layer.scale;
		var translate_x = this.options.layer.translate_x;
		var translate_y = this.options.layer.translate_y;
		
		var offset_x = -this.options.width / 2 + this.options.offset_x;
		var offset_y = -this.options.height / 2 + this.options.offset_y;

		for (var i = 0; i < len; i += 2) {
			var cog = this.markers[i / 2].cog * 0.017453292519943295;
			var x = translate_x + xy[i] * scale;
			var y = translate_y - xy[i + 1] * scale;
			ctx.translate(x, y);
			ctx.rotate(cog);
			ctx.drawImage(image, offset_x, offset_y);
			ctx.rotate(-cog);
			ctx.translate(-x, -y);
		}
	}
};
                      
MarkerArrayCanvas.prototype.lookup = function(x, y) 
{
	var image = this.image;

	var result;

	if (image.loaded) {

		var ctx = this.ctx;
		var xy	= this.xy_data;
		var len	= this.xy_len;

		var scale = this.options.layer.scale;
		var translate_x = this.options.layer.translate_x;
		var translate_y = this.options.layer.translate_y;
		
		var offset_x = -this.options.width / 2 + this.options.offset_x;
		var offset_y = -this.options.height / 2 + this.options.offset_y;

		translate_x += offset_x;
		translate_y += offset_y;

		for (var i = 0; i < len; i += 2) {

			var sx = xy[i] * scale + translate_x;
			var sy = translate_y - xy[i + 1] * scale;

			if ((x >= sx)&&(x <= sx + this.options.width)&&(y >= sy)&&(y <= sy + this.options.height)) {

				var image_data = ctx.getImageData(x, y, 1, 1);
	
				image_data.data[0] = 0;
				image_data.data[1] = 0;
				image_data.data[2] = 0;
				image_data.data[3] = 255;
		
				ctx.putImageData(image_data, x, y);

				ctx.drawImage(image, sx, sy);

				var image_data = ctx.getImageData(x, y, 1, 1).data;

				if ((image_data[0] != 0)||(image_data[1] != 0)||(image_data[2] != 0))
					result = i / 2;
			}			
		}
	}

	return result;
};

MarkerArrayCanvas.prototype.getPopup = function(vertex_index) 
{
	var marker = this.markers[vertex_index];

	return marker.popup_text(vertex_index);
};                  

MarkerArrayCanvas.prototype.beginUpdate = function() 
{	
	this.updating = true;
};

MarkerArrayCanvas.prototype.endUpdate = function() 
{
	var markers			= this.markers;

	var markers_count	= markers.length;

	var xy_data			= new Uint32Array(markers_count * 2);

	var position_index	= 0;
	for (var i = 0; i < markers_count; i++) {

		var marker = markers[i];

		xy_data[position_index++] = 6378137 * marker.lng * (Math.PI/180);
		xy_data[position_index++] = 6378137 * Math.log(Math.tan(Math.PI/4+marker.lat*(Math.PI/180)/2));
	}

	this.xy_data	= xy_data;
	this.xy_len		= position_index;

	this.updating	= false;
};