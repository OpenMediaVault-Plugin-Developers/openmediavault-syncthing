/**
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    OpenMediaVault Plugin Developers <plugins@omv-extras.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
 * @copyright Copyright (c) 2015-2016 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
// require("js/omv/WorkspaceManager.js")
// require("js/omv/workspace/form/Panel.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")

Ext.define("OMV.module.user.service.syncthing.Settings", {
    extend   : "OMV.workspace.form.Panel",
    uses     : [
        "OMV.data.Model",
        "OMV.data.Store"
    ],

    rpcService   : "Syncthing",
    rpcGetMethod : "getSettings",
    rpcSetMethod : "setSettings",

    plugins      : [{
        ptype        : "linkedfields",
        correlations : [{
            name : [
                "enable"
            ],
            conditions : [{
                name  : "enable",
                value : true
            }],
            properties : function(valid, field) {
                this.setButtonDisabled("openweb", !valid);
            }
        }]
    }],

    getButtonItems : function() {
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
            handler  : Ext.Function.bind(me.onOpenWebButton, me, [ me ])
        });
        return items;
    },

    getFormItems : function() {
        var me = this;
        return [{
            xtype    : "fieldset",
            title    : "General settings",
            defaults : {
                labelSeparator : ""
            },
            items : [{
                xtype      : "checkbox",
                name       : "enable",
                fieldLabel : _("Enable"),
                checked    : true
            },{
                xtype      : "textfield",
                name       : "username",
                fieldLabel : _("User"),
                readOnly   : true,
                hidden     : true
            },{
                xtype      : "textfield",
                name       : "uuid",
                fieldLabel : _("UUID"),
                readOnly   : true,
                hidden     : true
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
            }]
        }];
    },

    onOpenWebButton : function() {
        var me = this;
        window.open("http://" + window.location.hostname + ":" + me.getForm().findField("port").getValue(), "_blank");
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "settings",
    path      : "/service/syncthing",
    text      : _("Settings"),
    position  : 10,
    className : "OMV.module.user.service.syncthing.Settings"
});
