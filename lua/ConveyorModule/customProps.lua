require "vstudio"
local p = premake
local vstudio = p.vstudio
local project = p.project
local config = p.config
local fileconfig = p.fileconfig
local tree = p.tree
local m = p.vstudio.vc2010

premake.api.register {
    name = "customprops",
    scope = "config",
    kind = "list:keyed:string",
    tokens = true,
}


local function addCustomProps(prj)
    if prj.customtargets == nil or #prj.customtargets == 0 then
        return
    end

    for cfg in project.eachconfig(prj) do
        if #cfg.customprops > 0 then
            m.propertyGroup(cfg, "CustomProperties")
            for _,props in ipairs(cfg.customprops) do
                for prop, value in pairs(props) do
                    m.element(prop, nil, value);
                end
            end
            p.pop('</PropertyGroup>')
        end
    end
end

premake.override(premake.vstudio.vc2010.elements, "project", function(base, prj)
    local calls = base(prj)
    table.insert(calls, addCustomProps)
    return calls
end)