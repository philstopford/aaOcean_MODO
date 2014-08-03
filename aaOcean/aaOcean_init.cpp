#include "aaOcean_init.h"

namespace aaOceanCommand		{	extern void	initialize ();	};
namespace aaOceanDeformer		{	extern void	initialize ();	};
namespace aaOceanTextureNamespace		{	extern void	initialize ();	};
namespace aaOceanChanModNameSpace		{	extern void	initialize ();	};

void initialize ()
{
    aaOceanCommand :: initialize ();
    aaOceanDeformer :: initialize ();
    aaOceanTextureNamespace :: initialize ();
    aaOceanChanModNameSpace :: initialize();
}
