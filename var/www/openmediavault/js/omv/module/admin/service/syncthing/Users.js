/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2015 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

Ext.define("OMV.module.admin.service.syncthing.User", {
    extend   : "OMV.workspace.window.Form",
    requires : [
        "OMV.workspace.window.plugin.ConfigObject"
    ],

    rpcService   : "Syncthing",
    rpcGetMethod : "getUser",
    rpcSetMethod : "setUser",
    plugins      : [{
        ptype : "configobject"
    }],

    width        : 600,

    getFormItems : function() {
        var me = this;
        return [{
            xtype      : "checkbox",
            name       : "enable",
            fieldLabel : _("Enable"),
            checked    : true
        },{
            xtype      : "usercombo",
            name       : "username",
            fieldLabel : _("User"),
            readOnly   : (me.uuid !== OMV.UUID_UNDEFINED),
            value      : ""
        },{
            xtype         : "numberfield",
            name          : "port",
            fieldLabel    : _("GUI Port"),
            vtype         : "port",
            minValue      : 1,
            maxValue      : 65535,
            allowDecimals : false,
            allowNegative : false,
            allowBlank    : false,
            value         : 8384,
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("Port to listen on.")
            }]
        },{
            xtype: "textfield",
            name: "address",
            fieldLabel: _("GUI Address"),
            vtype: "IPv4Net",
            allowBlank: false,
            value: "127.0.0.1",
            plugins: [{
                ptype: "fieldinfo",
                text: _("IP address for GUI. Use 0.0.0.0 for all host IPs.")
            }]
        },{
            xtype         : "numberfield",
            name          : "lport",
            fieldLabel    : _("Listen Port"),
            vtype         : "port",
            minValue      : 1,
            maxValue      : 65535,
            allowDecimals : false,
            allowNegative : false,
            allowBlank    : false,
            value         : 22000,
            plugins    : [{
                ptype : "fieldinfo",
                text  : _("Port to listen on.")
            }]
        },{
            xtype: "textfield",
            name: "laddress",
            fieldLabel: _("Listen Address"),
            vtype: "IPv4Net",
            allowBlank: false,
            value: "0.0.0.0",
            plugins: [{
                ptype: "fieldinfo",
                text: _("IP address to listen on. Use 0.0.0.0 for all host IPs.")
            }]
        },{
            xtype: "numberfield",
            name: "maxsend",
            fieldLabel: _("Max Send"),
            allowDecimals: false,
            allowNegative: false,
            allowBlank: false,
            value: 0,
            plugins: [{
                ptype: "fieldinfo",
                text: _("Limit send speed. Value is Kb/s. 0 to disable.")
            }]
        },{
            xtype: "numberfield",
            name: "maxrecv",
            fieldLabel: _("Max Receive"),
            allowDecimals: false,
            allowNegative: false,
            allowBlank: false,
            value: 0,
            plugins: [{
                ptype: "fieldinfo",
                text: _("Limit receive speed. Value is Kb/s. 0 to disable.")
            }]
        }];
    }
});

Ext.define("OMV.module.admin.service.syncthing.Users", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.Rpc",
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.util.Format"
    ],
    uses     : [
        "OMV.module.admin.service.syncthing.User"
    ],

    hidePagingToolbar : false,
    stateful          : true,
    stateId           : "a982a76d-6804-1632-b31b-8b48c0ea6dde",
    columns           : [{
        xtype     : "booleaniconcolumn",
        text      : _("Enabled"),
        sortable  : true,
        dataIndex : "enable",
        stateId   : "enable",
        align     : "center",
        width     : 80,
        resizable : false,
        trueIcon  : "switch_on.png",
        falseIcon : "switch_off.png"
    },{
        text      : _("User"),
        sortable  : true,
        dataIndex : "username",
        stateId   : "username"
    },{
        text      : _("GUI Port"),
        sortable  : true,
        dataIndex : "port",
        stateId   : "port"
    },{
        text      : _("GUI Address"),
        sortable  : true,
        dataIndex : "address",
        stateId   : "address"
    },{
        text      : _("Listen Port"),
        sortable  : true,
        dataIndex : "lport",
        stateId   : "lport"
    },{
        text      : _("Listen Address"),
        sortable  : true,
        dataIndex : "laddress",
        stateId   : "laddress"
    },{
        text      : _("Max Send"),
        sortable  : true,
        dataIndex : "maxsend",
        stateId   : "maxsend"
    },{
        text      : _("Max Receive"),
        sortable  : true,
        dataIndex : "maxrecv",
        stateId   : "maxrecv"
    }],

    initComponent: function() {
        var me = this;
        Ext.apply(me, {
            store : Ext.create("OMV.data.Store", {
                autoLoad : true,
                model    : OMV.data.Model.createImplicit({
                    idProperty : "uuid",
                    fields     : [
                        { name : "uuid", type: "string" },
                        { name : "enable", type: "boolean" },
                        { name : "username", type: "string" },
                        { name : "port", type: "integer" },
                        { name : "address", type: "string" },
                        { name : "lport", type: "integer" },
                        { name : "laddress", type: "string" },
                        { name : "maxsend", type: "integer" },
                        { name : "maxrecv", type: "integer" }

                    ]
                }),
                proxy    : {
                    type    : "rpc",
                    rpcData : {
                        service : "Syncthing",
                        method  : "getUsers"
                    }
                }
            })
        });
        me.callParent(arguments);
    },

    getTopToolbarItems : function() {
        var me = this;
        var items = me.callParent(arguments);
        items.push({
            id       : me.getId() + "-openweb",
            xtype    : "button",
            text     : _("Open Web Interface"),
            icon     : "images/syncthing.png",
            iconCls  : Ext.baseCSSPrefix + "btn-icon-16x16",
            disabled : true,
            scope    : me,
            handler  : Ext.Function.bind(me.onOpenWebButton, me, [ me ]),
            disabled : true,
            selectionConfig : {
                minSelections : 1,
                maxSelections : 1
            }
        });
        return items;
    },

    onAddButton : function() {
        var me = this;
        Ext.create("OMV.module.admin.service.syncthing.User", {
            title     : _("Add user"),
            uuid      : OMV.UUID_UNDEFINED,
            listeners : {
                scope  : me,
                submit : function() {
                    this.doReload();
                }
            }
        }).show();
    },

    onEditButton : function() {
        var me = this;
        var record = me.getSelected();
        Ext.create("OMV.module.admin.service.syncthing.User", {
            title     : _("Edit user"),
            uuid      : record.get("uuid"),
            listeners : {
                scope  : me,
                submit : function() {
                    this.doReload();
                }
            }
        }).show();
    },

    doDeletion : function(record) {
        var me = this;
        OMV.Rpc.request({
            scope    : me,
            callback : me.onDeletion,
            rpcData  : {
                service : "Syncthing",
                method  : "deleteUser",
                params  : {
                    uuid : record.get("uuid")
                }
            }
        });
    },

    onOpenWebButton : function() {
        var record = this.getSelected();
        enable = record.get("enable");
        port = record.get("port");
        if (enable == true) {
            window.open("http://" + window.location.hostname + ":" + port, "_blank");
        }
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "users",
    path      : "/service/syncthing",
    text      : _("Syncthing Users"),
    position  : 10,
    className : "OMV.module.admin.service.syncthing.Users"
});
