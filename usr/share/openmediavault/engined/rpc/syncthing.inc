<?php
/**
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (C) 2009-2014 Volker Theile
 * @copyright Copyright (C) 2015-2016 OpenMediaVault Plugin Developers
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

class OMVRpcServiceSyncthing extends \OMV\Rpc\ServiceAbstract
{
    private $getLogPath = "/var/log/syncthing";

    public function getName()
    {
        return 'syncthing';
    }

    public function initialize()
    {
        // Admin functions
        $this->registerMethod('getUserList');
        $this->registerMethod('getUser');
        $this->registerMethod('setUser');
        $this->registerMethod('deleteUser');
        $this->registerMethod('getLogView');
        $this->registerMethod('doRestart');

        // User functions
        $this->registerMethod('getSettings');
        $this->registerMethod('setSettings');
        $this->registerMethod('getFolders');
        $this->registerMethod('getLog');
    }

    // Admin functions

    public function getUserList($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.getlist");
        // Get configuration data.
        $db = \OMV\Config\Database::getInstance();
        $objects = $db->getAssoc("conf.service.syncthing.user");
        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getUser($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        return $db->getAssoc("conf.service.syncthing.user", $params['uuid']);
    }

    public function setUser($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.syncthing.setuser");
        // Prepare configuration data.
        $object = new \OMV\Config\ConfigObject("conf.service.syncthing.user");
        $object->setAssoc($params);
        $isNew = $object->isNew();
        $db = \OMV\Config\Database::getInstance();
        if (TRUE === $isNew) {
            // Check uniqueness - port, lport
            $db->assertIsUnique($object, "port");
            $db->assertIsUnique($object, "lport");
        }
        $db->set($object);
        // Notify configuration changes.
        $dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
        $dispatcher->notify(
            $isNew ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.syncthing.user",
            $object->getAssoc()
        );
        // Return the configuration object.
        return $object->getAssoc();
    }

    public function deleteUser($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.objectuuid");
        // Delete the configuration object.
        return $this->deleteConfigObjectByUuid(
            "conf.service.syncthing.user",
            $params['uuid'],
            "org.openmediavault.services.syncthing.user"
        );
    }

    public function getLogView($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.common.getlist");

        $username = $params["username"];
        $logRows  = [];
        $logPath  = sprintf("%s/syncthing_%s.log", $this->getLogPath, $username);

        if (file_exists($logPath)) {
            $logText      = file_get_contents($logPath);
            $logTextArray = explode("\n", $logText);
            $rowNumber    = 1;

            foreach ($logTextArray as $row) {
                if (empty($row)) {
                    continue;
                }
                $logRows[] = [
                    "id"      => $rowNumber++,
                    "message" => $row,
                ];
            }
        }
        return [
            "total" => count($logRows),
            "data"  => $logRows,
        ];
    }

    public function doRestart($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_ADMINISTRATOR]);
        $db = \OMV\Config\Database::getInstance();
        if ( $db->exists("conf.service.syncthing.user", ["operator" => "stringEquals","arg0" => "enable" , arg1 => "1"])) {
            $systemCtl = new \OMV\System\SystemCtl("syncthing");
            $systemCtl->restart();
        }
    }


    // User functions

    public function getSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_EVERYONE]);
        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->getByFilter("conf.service.syncthing.user", [
            "operator" => "stringEquals",
            "arg0" => "username",
            "arg1" => $context['username']
        ],1);
        return $object->getAssoc();
    }

    public function setSettings($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_EVERYONE]);
        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, "rpc.syncthing.setuser");
        // Prepare configuration data.
        $object = new \OMV\Config\ConfigObject("conf.service.syncthing.user");
        $object->setAssoc($params);
        $isNew = $object->isNew();
        $db = \OMV\Config\Database::getInstance();
        if (TRUE === $isNew) {
            // Check uniqueness - port, lport
            $db->assertIsUnique($object, "port");
            $db->assertIsUnique($object, "lport");
        }
        $db->set($object);
        // Create/modify syncthing instance for user
        $cmd = sprintf("export LANG=C; omv-mkconf syncthing %s", $context['username']);
        if (0 !== $this->exec($cmd, $output)) {
            throw new \OMV\Exception(implode("\n", $output));
        }
        // Return the configuration object.
        return $object->getAssoc();
    }

    function getFolders($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_EVERYONE]);

        // Get the configuration object.
        $db = \OMV\Config\Database::getInstance();
        $object = $db->getByFilter("conf.service.syncthing.user", [
            "operator" => "stringEquals",
            "arg0" => "username",
            "arg1" => $context['username']
        ],1);
        $result = [];
        if (!is_null($object)) {
            $port    = intval($object->get('port'));
            $config  = file_get_contents(sprintf("http://localhost:%d/rest/system/config", $port));
            $configd = json_decode($config);

            foreach ($configd->folders as $folder) {
                if ($folder->id == "default") {
                    continue;
                }
                $result[] = [
                    "id"   => $folder->id,
                    "path" => $folder->path
                ];
            }
        }
        return $result;
    }

    public function getLog($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext($context, ["role" => OMV_ROLE_EVERYONE]);

        $output = "";
        $result = "";
        $cmd = sprintf("cat %s/syncthing_%s.log", $this->getLogPath, $context['username']);
        exec($cmd, $result);
        $output = implode("\n", $result) . "\n";
        return $output;
    }
}
