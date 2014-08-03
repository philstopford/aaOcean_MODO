/*
* aaOcean
 */
#include <lxu_command.hpp>
#include <lxu_select.hpp>
#include <lx_listener.hpp>
#include <lx_action.hpp>
#include <lx_item.hpp>
#include <lx_mesh.hpp>
#include <lx_deform.hpp>
#include <lx_layer.hpp>
#include <lxidef.h>
#include <string>
#include <vector>
#include <set>

// not used, but included to make it part of the SDK
#include <lx_undo.hpp>

namespace aaOceanCommand {	// disambiguate everything with a namespace


#define MORPHDEF_ITEMTYPE		"aaOceanDeform"
#define Cs_MORPH_MAPNAME		LXsICHAN_MORPHDEFORM_MAPNAME
#define ARGi_MAP		 0
static CLxItemType	morphDeformType (MORPHDEF_ITEMTYPE);


/*
 * ----------------------------------------------------------------
 * The selection tracker class keeps track of when item selection changes and
 * takes care of building a list of unique item types for the current state.
 * It's also in charge of enumeration, which is why we have a vistor class.
 */
class CItemVisitor
{
    public:
        virtual bool	Item (CLxUser_Item &item) = 0;
};

class CItemSelectionTracker :
                public CItemVisitor,
                public CLxImpl_SelectionListener,
                public CLxSingletonPolymorph
{
    public:
        CLxUser_SceneService		 srv_scene;
        CLxUser_SelectionService	 srv_sel;
        CLxUser_ItemPacketTranslation	 pkt_item;
        LXtID4				 selID_item;
        bool				 is_valid;
        unsigned			 use_count;
        std::set<LXtItemType> cur_types;

        LXxSINGLETON_METHOD;

        CItemSelectionTracker ()
        {
            is_valid  = false;
            use_count = 1;

            AddInterface (new CLxIfc_SelectionListener<CItemSelectionTracker>);

            selID_item = srv_sel.LookupType ("item");
            pkt_item.autoInit ();
        }

		void selevent_Add (LXtID4 type, unsigned int subtype) LXx_OVERRIDE
        {
            if (type == selID_item)
                    is_valid = false;
        }

		void selevent_Remove (LXtID4 type, unsigned int subtype) LXx_OVERRIDE
        {
            if (type == selID_item)
                    is_valid = false;
        }

		void Enumerate ( CItemVisitor		&vis)
        {
            CLxUser_Item		 item;
            LXtScanInfoID		 scan;
            void			*pkt;

            scan = 0;
            while ((scan = srv_sel.ScanLoopCurrent (scan, selID_item, &pkt)))
			{
                    pkt_item.GetItem (pkt, item);
                    if (vis.Item (item))
                            return;
            }
        }

		void ValidateTypeSet ()
        {
            cur_types.clear ();
            Enumerate (*this);
        }

		bool Item (CLxUser_Item		&item)
        {
            cur_types.insert (item.Type ());
            return false;
        }

		bool AllowType (LXtItemType	type)
        {
            std::set<LXtItemType>::iterator	 sit;

            if (!is_valid)
                    ValidateTypeSet ();

            for (sit = cur_types.begin(); sit != cur_types.end(); sit++)
                    if (srv_scene.ItemTypeTest (*sit, type) == LXe_TRUE)
                            return true;

            return false;
        }
};


/*
 * ----------------------------------------------------------------
 * The command has the usual collection of basic methods,
 * plus a method for customizing argument UI.
 */
class CMorphDefCommand : public CLxBasicCommand
{
public:
	CMorphDefCommand ();
	~CMorphDefCommand ();

	int  basic_CmdFlags	() LXx_OVERRIDE;
	bool basic_Notifier	(int index, std::string &name, std::string &args) LXx_OVERRIDE;
	bool basic_Enable	(CLxUser_Message &msg) LXx_OVERRIDE;

	CLxDynamicUIValue *atrui_UIValue	(unsigned int index);

	void		cmd_Execute	(unsigned int flags) LXx_OVERRIDE;
	LxResult	cmd_Query	(unsigned int index, ILxUnknownID vaQuery) LXx_OVERRIDE;

	CLxUser_SceneService		 srv_scene;
};


/*
 * Query -- return a list of all selected item name channel values 
 * perhaps this should also filtered by the value of the type
 */
class CQueryItemVisitor : public CItemVisitor
{
public:
    CLxUser_ValueArray	 va;

    bool Item (CLxUser_Item &item)
    {
        CLxUser_Scene		 scene;
        CLxUser_ChannelRead	 rchan;
        unsigned		 index;
        const char		*name = NULL;

        item.GetContext (scene);
        scene.GetChannels (rchan, 0.0);
        if (LXx_OK (item.ChannelLookup (Cs_MORPH_MAPNAME, &index)))
                if (LXx_OK (rchan.String (item, index, &name))) // HA, this looks like a User class method with a bool return, but it falls through to the LXe return local method.
                        va.Add (name);

        return false;
    }
};


class CMorphPopup : public CLxDynamicUIValue
{
    public:
        int				 wgtType;
        std::vector<CLxUser_Item>	 meshes;
        std::vector<std::string>	 morfNames;

        unsigned	Flags ()LXx_OVERRIDE
        {
			return LXfVALHINT_POPUPS;
        }

        unsigned	PopCount ()LXx_OVERRIDE
        {
			return (unsigned)morfNames.size ();
        }

        const char*	PopUserName (unsigned index)LXx_OVERRIDE
        {
            if(index>=morfNames.size ())return "OOB";
            return morfNames[index].c_str ();
        }

        const char*	PopInternalName (unsigned index) LXx_OVERRIDE
        {
            if(index>=morfNames.size ()) return "OOB";
            return morfNames[index].c_str ();
        }

        int		BuildNameLists ();
};

/*------------------------------------------------------------------------ 04/11
 * Mesh map visitor. Builds an array of mesh map names, which can be
 * used to select the maps to enable iterating over their elements.
 *----------------------------------------------------------------------------*/
class MeshMapVisitor : public CLxImpl_AbstractVisitor
{
    CLxUser_MeshMap*	m_meshMap;
    std::set<std::string>	m_mapNames;

    virtual LxResult	Evaluate ()
    {
            const char*	mapName;
            if (LXx_OK (m_meshMap->Name (&mapName))) 
			{
                    m_mapNames.insert (std::string (mapName));
            }
            return LXe_OK;
    }

public:
    MeshMapVisitor (CLxUser_MeshMap *MeshMap)
    {
            m_meshMap = MeshMap;
    }

    std::set<std::string>	GetMapNames ()	{ return m_mapNames; }
    void			ClearNames () { m_mapNames.clear (); }
};

/*
 * Execute -- this sets the map name for all selected morph items.
 */
class CExecItemVisitor : public CItemVisitor
{
    public:
        std::string		 mapName;
        unsigned		 nameIdx;

        bool Item (CLxUser_Item &item)
        {
            CLxUser_ChannelWrite	 chan;
            CLxUser_Scene		 scene;

            item.GetContext (scene);
            scene.SetChannels (chan, LXs_ACTIONLAYER_EDIT, 0.0);
            if (LXx_OK (item.ChannelLookup (Cs_MORPH_MAPNAME, &nameIdx)))
            {
                    if(mapName=="(none)")
                    {
                            chan.Set (item, nameIdx, "");
                    } else {
                            chan.Set (item, nameIdx, mapName.c_str());
                    }
            }
            return false;
        }
};


/*
 * Create command. Set the morph to the selected map, if any.
 */
class CCreateCommand
                : public CLxBasicCommand,
                  public CLxVertexMapSelection
{
public:
    CLxSceneSelection		 scene_sel;

	int basic_CmdFlags ()LXx_OVERRIDE
    {
		return LXfCMD_MODEL | LXfCMD_UNDO;
    }

	void cmd_Execute (unsigned int flags)LXx_OVERRIDE
    {
            CLxUser_Scene		 scene;
            CLxUser_Item		 item;
            std::string		 name;

            if (!scene_sel.Get (scene))
                    throw (LXe_NOTFOUND);

            if (!scene.NewItem (morphDeformType, item))
                    throw (LXe_FAILED);

            if (!First (name))
                    return;

            CLxUser_ChannelWrite	 write;

            write.setupFrom (scene);
            write.Set (item, Cs_MORPH_MAPNAME, name.c_str ());
    }

	bool Include (LXtID4 type) LXx_OVERRIDE
    {
            return (type == LXi_VMAP_TEXTUREUV);
    }
};

/*
 * Setup our command as a server. It has a command interface, an attributes
 * interface for arguments, and an attributesUI interface.
 */
void initialize ()
{
        CLxGenericPolymorph	*srv;

        srv = new CLxPolymorph<CMorphDefCommand>;
        srv->AddInterface (new CLxIfc_Command     <CMorphDefCommand>);
        srv->AddInterface (new CLxIfc_Attributes  <CMorphDefCommand>);
        srv->AddInterface (new CLxIfc_AttributesUI<CMorphDefCommand>);
        lx::AddServer ("aaOcean.mapName", srv);

        srv = new CLxPolymorph<CCreateCommand>;
        srv->AddInterface (new CLxIfc_Command     <CCreateCommand>);
        srv->AddInterface (new CLxIfc_Attributes  <CCreateCommand>);
        srv->AddInterface (new CLxIfc_AttributesUI<CCreateCommand>);
        lx::AddServer ("aaOcean.create", srv);
}


}