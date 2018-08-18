#include <stdio.h>
#include <stdlib.h>
#include <string.h>
	   
#define ABS(x) ((x)?(x):(x))

#define SET_TMP_END(tmpc,tmpp,src) {(tmpc)=*(src);(tmpp)=(src);*(src)='\0';}
#define RESTOR_TMP_END(tmpc,tmpp) {*(tmpp)=(tmpc);}

/*假设为一个ＨＨＴＰ报文头数据*/
char * buf="GET /service/open/nick?users=&callback=jQuery17204034407522995025_1386320955034&_=1386320955173 HTTP/1.1\r\nHost: api.csdn.net.cn\r\nConnection: keep-alive\r\nAccept: */*\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36\r\nReferer: http://bbs.csdn.net/home\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: zh-CN,zh;q=0.8\r\nCookie: 6320938.ovid\r\n\r\nHTTP/1.1 200 OK\r\nCache-Control: private\r\nContent-Length: 0";
char url[1024]={0};


char * International_Top_Level_Domain=".com.net.org.gov.edu.mil.biz.name.info.mobi.pro.travel.museum.int.aero.post.rec.asia.arts.firm.nom.store.web";

char * Country_Code_Top_Level_Domain = ".cn.tw.om.hk.ac.ad.ae.af.ag.ai.al.am.an.ao.aq.ar.as.at.au.aw.az.ba.bb.bd.be.bf.bg.bh.bi.bj.bm.bn.bo.br.bs.bt.bv.bw.by.bz.ca.cc.cf.cd.ch.ci.ck.cl.cm.co.cq.cr.cu.cv.cx.cy.cz.de.dj.dk.dm.do.dz.ec.ee.eg.eh.er.es.et.ev.fi.fj.fk.fm.fo.fr.ga.gd.ge.gf.gg.gh.gi.gl.gm.gn.gp.gr.gs.gt.gu.gw.gy.hm.hn.hr.ht.hu.id.ie.il.im.in.io.iq.ir.is.it.jm.jo.jp.je.ke.kg.kh.ki.km.kn.kp.kr.kw.ky.kz.la.lb.lc.li.lk.lr.ls.lt.lu.lv.ly.ma.mc.md.me.mg.mh.mk.ml.mm.mn.mo.mp.mq.mr.ms.mt.mu.mv.mw.mx.my.mz.na.nc.ne.nf.ng.ni.nl.no.np.nr.nt.nu.nz.qa.pa.pe.pf.pg.ph.pk.pl.pm.pn.pr.pt.pw.py.re.rs.ro.ru.rw.sa.sb.sc.sd.se.sg.sh.si.sj.sk.sl.sm.sn.so.sr.st.sv.su.sy.sz.sx.tc.td.tf.tg.th.tj.tk.tl.tm.tn.to.tr.tt.tv.tz.ua.ug.uk.um.us.uy.uz.va.vc.ve.vg.vi.vn.vu.wf.ws.ye.yt.za.zm.zw";
char* get_Second_Level_Domain(char *dest)
{
	
	char * p = NULL;
	char tmpc, *tmpp;
	
	/*过滤顶级域名*/
	p = strrchr(dest, '.');
	if(strlen(p) == 3)
	{
		/*过滤一级国际顶级域名*/
		SET_TMP_END(tmpc,tmpp,p);
		p = strrchr(dest, '.');
		RESTOR_TMP_END(tmpc,tmpp);
	}
	/*过滤二级域名*/
	
	SET_TMP_END(tmpc,tmpp,p);
	p = strrchr(dest, '.');
	RESTOR_TMP_END(tmpc,tmpp);

	return p+1;

}

int main(int argc, char** argv)
{
	char *p = buf;
	char *tbuf0 ;
	char *tbuf1;
	int tk,i;

	if(strlen(p)<15)
	{
		return 0;
	}
	url[0]='\0';
	
	/*判断每人上\r\n前边是不是http协议版本号*/
	p = memchr(p,'\r',strlen(p));
	tk = memcmp(p-8, "HTTP/1." ,7);
	if( tk != 0)
	{
		printf("skb is not http!\n");
		return 0;
	}

	p =buf;
	while(1)
	{
		tbuf0 = p;
		p = memchr(tbuf0,'\r',strlen(tbuf0));
		if(*(p+1) == '\n')
		{
			/*判断是不是报文头结束\r\n\r\n*/
			if(*(p+2) == '\r')
			{
				break;
			}
			p = p+2;

			
			tk = memcmp(p, "Host:" ,5);
			if( tk == 0)
			{	/*获取host字段内容*/
				tbuf0 = p+5;
				tbuf1= memchr(tbuf0,'\r',strlen(tbuf0));
				strncpy(url,p+5,ABS(tbuf1-tbuf0));
				
				printf("%s\n",get_Second_Level_Domain(url));
				
				break;
			}
			/*获取其他字段内容
			tk = memcmp(p, "Referer:" ,8);
			if( tk == 0)
			{	
				tbuf0 = p+8;
				tbuf1= memchr(tbuf0,'\r',strlen(tbuf0));
				strncpy(url,p+8,ABS(tbuf1-tbuf0));				
				break;
			}
		
			*/
		}
			
			
	}

	printf("url = %s\n",url);
	return 0;
}