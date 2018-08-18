inline void g_debug_set(int module)
{
  g_debug_switch |= module;
}

inline void g_debug_clear(int module)
{
  g_debug_switch &= ~module;
}

inline char *g_debug_module_name(int module)
{
  switch (module)
  {
    case NL_DEBUG_CORE:
      return "CORE";
    case NL_DEBUG_MEM:
      return "MEM";
    case NL_DEBUG_APP:
      return "APP";
    case NL_DEBUG_EVENT:
      return "EVENT";
	case NL_DEBUG_POLICYMGR:
      return "POLICYMGR";
    default:
      return "Unknown";
  }
}

int g_debug_switch = 0;
//分模块调试,写到屏幕
#define G_DEBUG(module, fmt, args...)                                     \
	if (g_debug_switch & module)                                    \
	{                                                                         \
		fprintf(stdout, "[%s] %s: "fmt,  g_debug_module_name(module), __FUNCTION__, ## args);   \
	}

	  
	  
	  
