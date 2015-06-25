<?php
/**
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (C) 2009-2014 Volker Theile
 * @copyright Copyright (C) 2015 OpenMediaVault Plugin Developers
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

require_once ("openmediavault/error.inc");
require_once ("openmediavault/initscript.inc");
require_once ("openmediavault/notify.inc");
require_once ("openmediavault/object.inc");
require_once ("openmediavault/rpc.inc");
require_once ("openmediavault/util.inc");

class OMVRpcServiceSyncthing extends OMVRpcServiceAbstract
{
    public function getName()
    {
        return 'syncthing';
    }

    private function getPluginName()
    {
        return strtolower( $this->getName() );
    }

    private function getXPath()
    {
        return sprintf( "//services/%s", $this->getPluginName() );
    }

    private function getXPathUsers()
    {
        return sprintf( "%s/users", $this->getXPath() );
    }

    private function getXPathUser()
    {
        return sprintf( "%s/user", $this->getXPathUsers() );
    }

    private function getEventMessagePath()
    {
        return sprintf( "org.openmediavault.services.%s", $this->getPluginName() );
    }

    private function getEventMessagePathUser()
    {
        return sprintf( "%s.users.user", $this->getEventMessagePath());
    }

    private function getLogPath()
    {
        return "/var/log/syncthing";
    }

    public function initialize()
    {
        // Admin functions
        $this->registerMethod('getUsers');
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

    public function getUsers($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "start"     : { "type" : "integer" },
                "limit"     : { '.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].' },
                "sortfield" : { '.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].' },
                "sortdir"   : { '.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].' }
            }
        }');

        // Get configuration data.
        $objects = $xmlConfig->getList( $this->getXPathUser() );
        if (is_null($objects))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $this->getXPathUser()
            );
        }

        // Filter the result.
        return $this->applyFilter($objects, $params['start'], $params['limit'],
            $params['sortfield'], $params['sortdir']);
    }

    public function getUser($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

          // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathUser(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }

        // Modify the result object.
        $object['enable']  = boolval($object['enable']);
        $object['port']    = intval($object['port']);
        $object['maxsend'] = intval($object['maxsend']);
        $object['maxrecv'] = intval($object['maxrecv']);

        return $object;
    }

    public function setUser($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid"     : { '.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].' },
                "enable"   : { "type" : "boolean" },
                "username" : { "type" : "string" },
                "port"     : { "type" : "integer" },
                "address"  : { "type" : "string" },
                "lport"    : { "type" : "integer" },
                "laddress" : { "type" : "string" },
                "maxsend"  : { "type" : "integer" },
                "maxrecv"  : { "type" : "integer" }
            }
        }');

        // Prepare configuration data.
        $object = array(
            "uuid"     => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $params['uuid'],
            "enable"   => array_boolval($params, 'enable'),
            "username" => $params['username'],
            "port"     => intval($params['port']),
            "address"  => $params['address'],
            "lport"    => intval($params['lport']),
            "laddress" => $params['laddress'],
            "maxsend"  => intval($params['maxsend']),
            "maxrecv"  => intval($params['maxrecv'])
        );

        // Set the configuration data.
        $success = FALSE;
        if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
        {
            $xPath = sprintf("%s[username='%s']", $this->getXPathUser(), $params['username']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This username is already in use.")
                );
            }

            $xPath = sprintf("%s[port='%s']", $this->getXPathUser(), $params['port']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This GUI port is already in use.")
                );
            }

            $xPath = sprintf("%s[lport='%s']", $this->getXPathUser(), $params['lport']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This listen port is already in use.")
                );
            }

            // Append object to configuration.
            $success = $xmlConfig->set($this->getXPathUsers(), array("user" => $object));
        }
        else
        {
            // Update existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathUser(), $params['uuid'] );
            $success = $xmlConfig->replace($xPath, $object);
        }
        if (FALSE === $success)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED
            );
        }

        // Notify configuration changes.
        $dispatcher = &OMVNotifyDispatcher::getInstance();
        $dispatcher->notify(
            ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
            $this->getEventMessagePathUser(),
            $object
        );

        // Return the configuration object.
        return $object;
    }

    public function deleteUser($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "uuid" : { '.$GLOBALS['OMV_JSONSCHEMA_UUID'].' }
            }
        }');

        // Get the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathUser(), $params['uuid']);
        $object = $xmlConfig->get($xPath);
        if (is_null($object))
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
                $xPath
            );
        }

        // Delete the configuration object.
        $xPath = sprintf("%s[uuid='%s']", $this->getXPathUser(), $params['uuid']);

        return $this->deleteConfigObjectByPath(
            $xPath,
            $this->getEventMessagePathUser()
        );
    }

    public function getLogView($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_ADMINISTRATOR)
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "start"     : { "type": "integer" },
                "limit"     : {' . $GLOBALS["OMV_JSONSCHEMA_COUNTFIELD"] . '},
                "sortfield" : {' . $GLOBALS["OMV_JSONSCHEMA_SORTFIELD"] . '},
                "sortdir"   : {' . $GLOBALS["OMV_JSONSCHEMA_SORTDIR"] . '},
                "username"  : { "type": "string" }
            }
        }');

        $username = $params["username"];
        $logRows  = array();
        $logPath  = sprintf("%s/syncthing_%s.log", $this->getLogPath(), $username);

        if (file_exists($logPath))
        {
            $logText      = file_get_contents($logPath);
            $logTextArray = explode("\n", $logText);
            $rowNumber    = 1;

            foreach ($logTextArray as $row)
            {
                if (empty($row))
                    continue;

                $logRows[] = array(
                    "id"      => $rowNumber++,
                    "message" => $row,
                );
            }
        }

        return array(
            "total" => count($logRows),
            "data"  => $logRows,
        );
    }

    public function doRestart($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array( "role" => OMV_ROLE_ADMINISTRATOR )
        );

        $xPath = sprintf("%s[enable=1]", $this->getXPathUser());
        if (TRUE === $xmlConfig->exists($xPath))
        {
            $initd = new OMVSysVInitScript($this->getPluginName());
            $initd->invoke("restart");
        }
    }


    // User functions

    public function getSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_USER)
        );

        // Get the configuration object.
        $xPath = sprintf("%s[username='%s']", $this->getXPathUser(), $context['username']);
        $object = $xmlConfig->get($xPath);

        // Modify the result object.
        if (!is_null($object))
        {
            $object['enable']   = boolval($object['enable']);
            $object['port']     = intval($object['port']);
            $object['maxsend']  = intval($object['maxsend']);
            $object['maxrecv']  = intval($object['maxrecv']);
        }

        $object['username'] = $context['username'];

        return $object;
    }

    public function setSettings($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_USER)
        );

        // Validate the parameters of the RPC service method.
        $this->validateMethodParams($params, '{
            "type"       : "object",
            "properties" : {
                "enable"   : { "type" : "boolean" },
                "username" : { "type" : "string" },
                "port"     : { "type" : "integer" },
                "address"  : { "type" : "string" },
                "lport"    : { "type" : "integer" },
                "laddress" : { "type" : "string" },
                "maxsend"  : { "type" : "integer" },
                "maxrecv"  : { "type" : "integer" }
            }
        }');

        $newUser = false;

        // Get the configuration object.
        $xPath = sprintf("%s[username='%s']", $this->getXPathUser(), $context['username']);
        $oldObject = $xmlConfig->get($xPath);

        if (is_null($oldObject))
            $newUser = true;

        // Prepare configuration data.
        $object = array(
            "uuid"     => ($newUser) ? OMVUtil::uuid() : $oldObject['uuid'],
            "enable"   => array_boolval($params, 'enable'),
            "username" => $params['username'],
            "port"     => intval($params['port']),
            "address"  => $params['address'],
            "lport"    => intval($params['lport']),
            "laddress" => $params['laddress'],
            "maxsend"  => intval($params['maxsend']),
            "maxrecv"  => intval($params['maxrecv'])
        );

        // Set the configuration data.
        $success = FALSE;
        if ($newUser === true)
        {
            $xPath = sprintf("%s[port='%s']", $this->getXPathUser(), $object['port']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This GUI port is already in use.")
                );
            }

            $xPath = sprintf("%s[lport='%s']", $this->getXPathUser(), $object['lport']);
            if (TRUE === $xmlConfig->exists($xPath))
            {
                throw new OMVException(
                    OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
                    gettext("This listen port is already in use.")
                );
            }

            // Append object to configuration.
            $success = $xmlConfig->set($this->getXPathUsers(), array("user" => $object));
        }
        else
        {
            // Update existing configuration object.
            $xPath = sprintf("%s[uuid='%s']", $this->getXPathUser(), $object['uuid'] );
            $success = $xmlConfig->replace($xPath, $object);
        }
        if (FALSE === $success)
        {
            throw new OMVException(
                OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED
            );
        }

        $cmd = sprintf("export LANG=C; omv-mkconf %s %s", $this->getPluginName(), $context['username']);
        if (0 !== $this->exec($cmd, $output))
        {
            throw new OMVException(
                OMVErrorMsg::E_EXEC_FAILED,
                $cmd,
                implode("\n", $output)
            );
        }
    }

    function getFolders($params, $context)
    {
        global $xmlConfig;

        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_USER)
        );

        $result  = array();

        // Get configuration object
        $xPath = sprintf("%s[username='%s']", $this->getXPathUser(), $context['username'] );
        $object = $xmlConfig->get($xPath);

        if (!is_null($object))
        {
            $port    = intval($object['port']);
            $config  = file_get_contents(sprintf("http://localhost:%d/rest/system/config", $port));
            $configd = json_decode($config);

            foreach ($configd->folders as $folder)
            {
                if ($folder->id == "default")
                    continue;

                $result[] = array(
                    "id"   => $folder->id,
                    "path" => $folder->path
                );
            }
        }

        return $result;
    }

    public function getLog($params, $context)
    {
        // Validate the RPC caller context.
        $this->validateMethodContext(
            $context,
            array("role" => OMV_ROLE_USER)
        );

        $output = "";
        $result = "";

        $cmd = sprintf("cat %s/syncthing_%s.log", $this->getLogPath(), $context['username']);
        OMVUtil::exec($cmd, $result);
        $output = implode("\n", $result) . "\n";

        return $output;
    }
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSyncthing());
