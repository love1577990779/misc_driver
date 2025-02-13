//#include <common.h>
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <linux/bug.h>
#include <linux/libfdt.h>
#include <dm/of_access.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/ioport.h>
#include <efi_loader.h>

int of_address_to_resource(const struct device_node *dev, int index,
			   struct resource *r)
{
    return 0;
}

int of_read_u32_index(const struct device_node *np, const char *propname,
		      int index, u32 *outp)
{
    return 0;
}

struct property *of_find_property(const struct device_node *np,
				  const char *name, int *lenp)
{
    return NULL;
}

int of_read_u32_array(const struct device_node *np, const char *propname,
		      u32 *out_values, size_t sz)
{
    return 0;
}

bool of_device_is_available(const struct device_node *device)
{
    return 0;
}

struct device_node *of_get_parent(const struct device_node *node)
{
    return NULL;
}

struct device_node *of_find_node_by_phandle(struct device_node *root,
					    phandle handle)
{
    return NULL;
}

const __be32 *of_get_address(const struct device_node *dev, int index,
			     u64 *size, unsigned int *flags)
{
    return NULL;
}

int of_n_size_cells(const struct device_node *np)
{
    return 0;
}

u64 of_translate_address(const struct device_node *dev, const __be32 *in_addr)
{
    return 0;
}

int of_n_addr_cells(const struct device_node *np)
{
    return 0;
}

int of_parse_phandle_with_args(const struct device_node *np,
			       const char *list_name, const char *cells_name,
			       int cell_count, int index,
			       struct of_phandle_args *out_args)
{
    return 0;
}

int of_count_phandle_with_args(const struct device_node *np,
			       const char *list_name, const char *cells_name,
			       int cell_count)
{
    return 0;
}

struct device_node *of_find_node_opts_by_path(struct device_node *root,
					      const char *path,
					      const char **opts)
{
    return NULL;
}

const void *of_get_property(const struct device_node *np, const char *name,
			    int *lenp)
{
    return 0;
}

int of_simple_addr_cells(const struct device_node *np)
{
    return 0;
}

int of_simple_size_cells(const struct device_node *np)
{
    return 0;
}

struct device_node *of_find_node_by_prop_value(struct device_node *from,
					       const char *propname,
					       const void *propval, int proplen)
{
    return NULL;
}

struct device_node *of_get_stdout(void)
{
    return NULL;
}

efi_status_t efi_add_memory_map_pg(u64 start, u64 pages,
	int memory_type,
	bool overlap_conventional)
{
	return 0;
}

static void
main_loop(void)
{
	return;
}