/* $Id$ */

/* Pluto - Heavy-duty persistence for Lua
 * Copyright (C) 2004 by Ben Sunshine-Hill, and released into the public 
 * domain. People making use of this software as part of an application
 * are politely requested to email the author at sneftel@gmail.com 
 * with a brief description of the application, primarily to satisfy his
 * curiosity.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* lua.h must be included before this file */

void pluto_persist(lua_State *L, lua_Chunkwriter writer, void *ud);

void pluto_unpersist(lua_State *L, lua_Chunkreader reader, void *ud);

int pluto_open(lua_State *L);

typedef struct WriterInfo_t {
	char* buf;
	size_t buflen;
}
WriterInfo;

int bufwriter (lua_State *L, const void* p, size_t sz, void* ud);

typedef struct LoadInfo_t {
	const char *buf;
	size_t size;
}
LoadInfo;

const char *bufreader(lua_State *L, void *ud, size_t *sz);
