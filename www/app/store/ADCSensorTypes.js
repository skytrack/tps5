/*
 * File: app/store/ADCSensorTypes.js
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

Ext.define('MyApp.store.ADCSensorTypes', {
	extend: 'Ext.data.Store',

	constructor: function(cfg) {
		var me = this;
		cfg = cfg || {};
		me.callParent([Ext.apply({
			storeId: 'MyStore2',
			data: [
				{
					id: 0,
					text: 'Общего назначения'
				},
				{
					id: 2,
					text: 'Датчик уровня топлива левый'
				},
				{
					id: 3,
					text: 'Датчик уровня топлива правый'
				}
			],
			fields: [
				{
					name: 'id',
					type: 'int'
				},
				{
					name: 'text',
					type: 'string'
				}
			]
		}, cfg)]);
	}
});