local p = premake
local vstudio = p.vstudio
local project = p.project
local config = p.config

local diagnosticLogging = false

premake.api.register {
    name = "exports",
    scope = "config",
    kind  = "list:keyed:list:string",
}

local function diagLog(msg)
    if diagnosticLogging then
        print(msg)
    end
end

function importLinkExports()
    local next = next
    local wks = premake.global.getWorkspace(1)
    if wks then
        for prj in p.workspace.eachproject(wks) do
            --if prj.name == "AssetBuilder" then
            --    diagnosticLogging = true
            --end

            diagLog(prj)
            diagLog(prj.name)

            for cfg in project.eachconfig(prj) do
                diagLog('\t' .. cfg.name)

                for _,export in ipairs(cfg.exports) do
                    for exportKey, exportValue in pairs(export) do
                        diagLog('\t\t\tImporting ' .. exportKey)
                        if cfg[exportKey] == nil then
                            cfg[exportKey] = {}
                        elseif type(cfg[exportKey]) ~= "table" then
                            error("Trying to append field " .. exportKey .. " to non-table")
                        end

                        if type(exportValue) == "table" then
                            cfg[exportKey] = table.concatenate(cfg[exportKey], exportValue)
                            diagLog(exportValue[1])
                        else
                            table.insert(cfg[exportKey], exportValue)
                            diagLog(exportValue)
                        end
                    end
                end

                local processedLinks = {}
                local linkTable = table.shallowCopy(cfg.links)
                if not linkTable then
                    goto skip_config
                end

                local i, v = next(linkTable, nil)
                while #linkTable > 0 do
                    local linkPrj = nil
                    local otherCfg = nil

                    table.remove(linkTable, i)
                    if processedLinks[v] ~= nil then
                        goto skip_link
                    end

                    processedLinks[v] = true
                    linkPrj = premake.workspace.findproject(wks, v)
                    if not linkPrj then
                        goto skip_link
                    end

                    otherCfg = premake.project.getconfig(linkPrj, cfg.buildcfg, cfg.platform)
                    diagLog('\t\tChecking ' .. v)
                    if not otherCfg or not otherCfg.exports then
                        goto skip_link
                    end

                    for _,export in ipairs(otherCfg.exports) do
                        for exportKey, exportValue in pairs(export) do
                            diagLog('\t\t\tImporting ' .. exportKey)
                            if cfg[exportKey] == nil then
                                cfg[exportKey] = {}
                            elseif type(cfg[exportKey]) ~= "table" then
                                error("Trying to append field " .. exportKey .. " to non-table")
                            end

                            if type(exportValue) == "table" then
                                cfg[exportKey] = table.concatenate(cfg[exportKey], exportValue)
                                diagLog(exportValue[1])

                                if exportKey == "links" then
                                    diagLog("\t\t\t\tAdding extra link")
                                    for _,vlink in pairs(cfg[exportKey]) do
                                        table.insert(linkTable, vlink)
                                    end
                                end
                            else
                                table.insert(cfg[exportKey], exportValue)
                                diagLog(exportValue)

                                if exportKey == "links" then
                                    diagLog("\t\t\t\tAdding extra link")
                                    table.insert(linkTable, cfg[exportKey])
                                end
                            end
                        end
                    end

                    ::skip_link::
                    i, v = next(linkTable, nil)
                end

                ::skip_config::
            end

            diagnosticLogging = false
        end
    end
end

premake.override(premake.main, 'postBake', function(base, prj)
    importLinkExports()
    base(prj)
end)
