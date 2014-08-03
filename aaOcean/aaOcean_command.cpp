
#include "aaOcean_command.h"
using namespace aaOceanCommand;

/*
 * We only need one selection tracker for however many instances of commands, so
 * we keep a count.
 */
static CItemSelectionTracker *sT = 0;

void SelTrack_Acquire (void)
{
    if (sT) 
	{
        sT->use_count++;
        return;
    }

    CLxUser_ListenerService	 ls;

    sT = new CItemSelectionTracker;
    ls.AddListener (*sT);
}

void SelTrack_Release (void)
{
    sT->use_count--;
    if (sT->use_count)
            return;

    CLxUser_ListenerService	 ls;

    ls.RemoveListener (*sT);
    delete sT;
    sT = 0;
}



CMorphDefCommand::CMorphDefCommand ()
{
    dyna_Add (Cs_MORPH_MAPNAME, LXsTYPE_STRING);
    basic_SetFlags (0, LXfCMDARG_QUERY);

    SelTrack_Acquire ();
}

CMorphDefCommand::~CMorphDefCommand ()
{
	SelTrack_Release ();
}


int CMorphDefCommand::basic_CmdFlags ()
{
	return LXfCMD_MODEL | LXfCMD_UNDO;
}


// This is probably important for gettng the popup to update.  Searching for docs or an example as yet, fruitless
bool CMorphDefCommand::basic_Notifier (int index, std::string &name, std::string &args)
{
    if (index == 0) 
	{
        //name = "channel.event";
        //args = "+d "Cs_MORPH_MAPNAME " " MORPHDEF_ITEMTYPE;
        name = "select.event";
        args = "item +v";
    }
    else
		return false;

    return true;
}


/*
 * Enable -- test if there's anything selected.
 */
class CEnableItemVisitor : public CItemVisitor
{
    public:
        bool any;

        bool Item (CLxUser_Item &item)
        {
            any = true;
            return true;
        }
};

bool CMorphDefCommand::basic_Enable (CLxUser_Message &msg)
{
    CEnableItemVisitor	 vis;

    vis.any = false;
    sT->Enumerate (vis);
    return vis.any;
}



LxResult CMorphDefCommand::cmd_Query (unsigned int index, ILxUnknownID vaQuery)
{
        CQueryItemVisitor	 vis;

        vis.va.set (vaQuery);
        sT->Enumerate (vis);
        return LXe_OK;
}


class CMeshListItemVisitor : public CItemVisitor
{
    public:
        std::vector<CLxUser_Item>	*meshes;
        CLxUser_DeformerService		 srv_deform;

        bool Item (CLxUser_Item &item)
        {
            unsigned		 n=0;
            int			 i;
            ILxItemID		 xitem;

            srv_deform.MeshCount (item, &n);
            for (i=0; i<n; i++) 
			{
                    if (LXx_OK (srv_deform.MeshByIndex (item, i, (void**)&xitem))) 
					{
                            CLxUser_Item	mesh;
                            mesh.set (xitem);
                            meshes->push_back (mesh);
                    }
            }

            return false;
        }
};


int CMorphPopup::BuildNameLists () 
{
        CMeshListItemVisitor		mlv;
        mlv.meshes = &meshes;
        sT->Enumerate (mlv); // build list of meshes

        CLxUser_Mesh		 umesh;
        CLxUser_MeshMap		 meshMap;
        CLxUser_ChannelRead	 rchan;
        CLxUser_Scene		 scene;

        std::string none("(none)");
        morfNames.push_back (none);

        unsigned		 index;
        std::vector<CLxUser_Item>::iterator it;
        for (it=meshes.begin(); it!=meshes.end(); it++) 
		{
                if (!LXx_OK(it[0].ChannelLookup (LXsICHAN_MESH_MESH, &index)))
                        continue;
                CLxUser_Item itm = *it;
                itm.GetContext (scene);
                scene.GetChannels (rchan, LXs_ACTIONLAYER_EDIT);
                if (!rchan.Object (itm, index, umesh))
                        continue;
                
				umesh.GetMaps (meshMap);

                MeshMapVisitor	nameVisitor (&meshMap);

                meshMap.FilterByType (LXi_VMAP_TEXTUREUV);
                meshMap.Enum (&nameVisitor);
                //meshMap.FilterByType (LXi_VMAP_SPOT);
                //meshMap.Enum (&nameVisitor);
                
                std::set<std::string>::iterator		strItr;
                std::set<std::string> nl = nameVisitor.GetMapNames ();
                for (strItr = nl.begin (); strItr!=nl.end (); strItr++) 
				{
                        morfNames.push_back (*strItr);
                }
        }
        return 0;
}



CLxDynamicUIValue *CMorphDefCommand::atrui_UIValue (unsigned int index)
{	
    CMorphPopup *wp = new CMorphPopup;
    wp->BuildNameLists ();
    return wp;
}


void CMorphDefCommand::cmd_Execute (unsigned int flags)
{
    CExecItemVisitor	 vis;

    if (!LXx_OK (attr_GetString (0, vis.mapName))) 
	{
            CLxUser_Message		&msg = basic_Message ();
            msg.SetMsg ("common", 99);
            msg.SetArg (1, "Please choose an morph deformer!");
            return;
    }
    sT->Enumerate (vis);
}









