//******************************************************************************
//
// File Name : Distance.js
// Author    : Skytrack ltd - Copyright (C) 2014
//
// This code is property of Skytrack company
//
//******************************************************************************

L.Distance = L.Class.extend({

    onAdd: function (map) {
    	this.map = map;

		map.on('mousemove', this.mousemove, this);
		map.on('click', this.click, this);

    },
	
	onRemove: function (map) {

        map.off("mousemove", this.mousemove, this);
		map.off("click", this.click, this);

		var polyline = this.polyline;

		if (typeof polyline != 'undefined') {
			
			map.removeLayer(polyline);

			delete this.polyline;
		}

		this.map.closePopup();
    },

	mousemove: function(event) {

		var polyline = this.polyline;

		if (typeof polyline != 'undefined') {

			polyline.spliceLatLngs(0, 1, event.latlng);

			if (typeof this.timeout != 'undefined') {
				clearTimeout(this.timeout);
				this.map.closePopup();
			}

			var self = this;

			this.timeout = setTimeout( 
				function() {
					var popup = L.popup()
						.setLatLng(event.latlng) 
						.setContent((self.length() / 1000).toFixed(2) + ' km')
						.openOn(self.map);
				},
				300
			);
		}
	},

	click: function(event) {

		var polyline = this.polyline;

		if (typeof polyline != 'undefined') {

			polyline.spliceLatLngs(0, 0, event.latlng);
		}
		else {

			this.polyline = L.polyline([event.latlng, event.latlng], {color: 'red'}).addTo(this.map);
		}
	},

	length: function() {

		var polyline = this.polyline;

		if (typeof polyline != 'undefined') {
			
			var latlngs = polyline.getLatLngs();

			var length = 0;

			var prev_latlng = latlngs[latlngs.length - 1];

			for (var i = latlngs.length - 1; i--;) {

				var latlng = latlngs[i];

				length += prev_latlng.distanceTo(latlng);

				prev_latlng = latlng;
			} 

			return length;
		}

		return 0;
	}
});

L.distance = function() {
    return new L.Distance();
};