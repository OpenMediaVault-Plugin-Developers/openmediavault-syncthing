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
// require("js/omv/workspace/grid/Panel.js")
// require("js/omv/workspace/window/Form.js")
// require("js/omv/workspace/window/plugin/ConfigObject.js")
// require("js/omv/util/Format.js")
// require("js/omv/Rpc.js")
// require("js/omv/data/Store.js")
// require("js/omv/data/Model.js")
// require("js/omv/data/proxy/Rpc.js")

Ext.define("OMV.module.user.service.syncthing.Folders", {
    extend   : "OMV.workspace.grid.Panel",
    requires : [
        "OMV.Rpc",
        "OMV.data.Store",
        "OMV.data.Model",
        "OMV.data.proxy.Rpc",
        "OMV.util.Format"
    ],

    hidePagingToolbar : false,
    hideAddButton     : true,
    hideEditButton    : true,
    hideDeleteButton  : true,
    stateful          : true,
    stateId           : "1a27a76d-6804-1332-b31b-8b48c0ea6dde",
    columns           : [{
        text      : _("ID"),
        sortable  : true,
        dataIndex : "id",
        stateId   : "id"
    },{
        text      : _("Path"),
        sortable  : true,
        dataIndex : "path",
        stateId   : "path"
    }],

    initComponent : function() {
        var me = this;
        Ext.apply(me, {
            store : Ext.create("OMV.data.Store", {
                autoLoad : true,
                model    : OMV.data.Model.createImplicit({
                    idProperty  : "id",
                    fields      : [
                        { name : "id", type : "string" },
                        { name : "path", type : "string" }
                    ]
                }),
                proxy : {
                    type    : "rpc",
                    rpcData : {
                        service : "Syncthing",
                        method  : "getFolders"
                    }
                }
            })
        });
        me.callParent(arguments);
    }
});

OMV.WorkspaceManager.registerPanel({
    id        : "folders",
    path      : "/service/syncthing",
    text      : _("Folders"),
    position  : 20,
    className : "OMV.module.user.service.syncthing.Folders"
});
