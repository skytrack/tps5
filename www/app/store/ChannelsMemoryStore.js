/*
 * File: app/store/ChannelsMemoryStore.js
 *
 * This file was generated by Sencha Architect version 2.2.3.
 * http://www.sencha.com/products/architect/
 *
 * This file requires use of the Ext JS 4.2.x library, under independent license.
 * License of Sencha Architect does not include license for Ext JS 4.2.x. For more
 * details see http://www.sencha.com/license or contact license@sencha.com.
 *
 * This file will be auto-generated each and everytime you save your project.
 *
 * Do NOT hand edit this file.
 */

Ext.define('MyApp.store.ChannelsMemoryStore', {
	extend: 'Ext.data.Store',

	constructor: function(cfg) {
		var me = this;
		cfg = cfg || {};
		me.callParent([Ext.apply({
			autoLoad: true,
			autoSync: true,
			storeId: 'channel_memory_store',
			proxy: {
				type: 'memory'
			},
			fields: [
				{
					name: 'text',
					type: 'string'
				},
				{
					name: 'name',
					type: 'string'
				},
				{
					defaultValue: false,
					name: 'binary',
					type: 'boolean'
				},
				{
					name: 'binary_bit'
				},
				{
					name: 'mask',
					type: 'string'
				},
				{
					name: 'mask_bit'
				},
				{
					name: 'data_field',
					type: 'string'
				},
				{
					defaultValue: 1,
					name: 'factor'
				},
				{
					defaultValue: true,
					name: 'visible'
				},
				{
					defaultValue: false,
					name: 'logical',
					type: 'boolean'
				},
				{
					name: 'presense_field'
				},
				{
					name: 'mask_bit_field',
					type: 'string'
				}
			]
		}, cfg)]);
	}
});