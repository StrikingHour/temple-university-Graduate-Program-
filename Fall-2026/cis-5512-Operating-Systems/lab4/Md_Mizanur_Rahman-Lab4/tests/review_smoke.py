#!/usr/bin/env python3
"""Local functional review; uses an ephemeral port, not a performance test."""
import os,pathlib,subprocess,socket,time,re
root=pathlib.Path(__file__).resolve().parents[1]
def build(*args):subprocess.run(args,check=True)
build('gcc','-O2','-I'+str(root/'include'),str(root/'tests/tuple_probe.c'),str(root/'obj/libsng.a'),'-o',str(root/'tests/tuple_probe'))
with socket.socket() as s:s.bind(('127.0.0.1',0));port=s.getsockname()[1]
env=dict(os.environ,TSHPORT=str(port),TSHHOST='127.0.0.1',MSH_PATH=str(root/'tsh/myshell'),TERM='dumb')
log=open(root/'evidence/review_daemon.txt','w')
daemon=subprocess.Popen([str(root/'tsh/tsh'),str(port)],env=env,cwd=root/'tsh',stdout=log,stderr=log)
children=[]
def cmd(rel,*args,expect=0,**kwargs):
 r=subprocess.run([str(root/rel),*args],env=env,cwd=root/'tools',capture_output=True,text=True,timeout=20,**kwargs)
 assert r.returncode==expect,(rel,args,r.stdout,r.stderr,r.returncode)
 return r.stdout
try:
 time.sleep(.2)
 cmd('tools/tshctl','shell','/bin/true');cmd('tools/tshctl','shell','/bin/false',expect=1)
 cmd('tools/tshctl','shell','/no_such_review_program',expect=127)
 cmd('tools/tshctl','shell','/bin/echo Hello | /usr/bin/wc -c > review_pipe.txt')
 assert (root/'tsh/review_pipe.txt').read_text().strip()=='6'
 print('PASS foreground exit status, missing executable, pipe/redirection')
 out=cmd('tools/tshctl','shell','/bin/sleep 20','--bg');pid=re.search(r'Job PID: (\d+)',out)[1]
 assert 'Active jobs: 1' in cmd('tools/tshctl','ps')
 cmd('tools/tshctl','kill',pid);time.sleep(.2)
 assert 'Active jobs: 0' in cmd('tools/tshctl','ps')
 cmd('tools/tshctl','kill','2147483647',expect=1)
 print('PASS background, PS, KILL and unknown PID rejection')
 for _ in range(2):cmd('tools/tshctl','shell','/bin/sleep 20','--bg')
 cmd('tools/tshctl','killall');time.sleep(.2)
 assert 'Active jobs: 0' in cmd('tools/tshctl','ps')
 print('PASS KILLALL and child reaping')
 cmd('tests/tuple_probe','put','review_tuple','first')
 for _ in range(2):assert cmd('tests/tuple_probe','read','^review_tuple$').rstrip('\x00')=='first'
 assert cmd('tests/tuple_probe','get','^review_tuple$').rstrip('\x00')=='first'
 blocked=subprocess.Popen([str(root/'tests/tuple_probe'),'get','^review_tuple$'],env=env,stdout=subprocess.PIPE);children.append(blocked)
 time.sleep(.2);assert blocked.poll() is None
 cmd('tests/tuple_probe','put','review_tuple','second')
 assert blocked.communicate(timeout=5)[0]==b'second\x00'
 print('PASS PUT, repeated READ, destructive GET and pending GET callback')
 blocked=subprocess.Popen([str(root/'tests/tuple_probe'),'read','^review_read$'],env=env,stdout=subprocess.PIPE);children.append(blocked)
 time.sleep(.2);assert blocked.poll() is None
 cmd('tests/tuple_probe','put','review_read','broadcast')
 assert blocked.communicate(timeout=5)[0]==b'broadcast\x00'
 assert cmd('tests/tuple_probe','get','^review_read$').rstrip('\x00')=='broadcast'
 print('PASS pending READ retains tuple for subsequent GET')
 cmd('tools/launch','demo_worker','0',expect=2);cmd('tools/launch','missing_worker','4',expect=1)
 print('PASS launcher invalid count and missing executable')
 for index,(n,g,p) in enumerate([(8,2,4),(7,3,3),(5,5,4),(256,17,4)]):
  run='review'+str(index)
  cmd('tools/launch',str(root/'matrix/worker'),str(p),run)
  result=cmd('matrix/client',str(n),str(g),str(p),run)
  assert 'Verification: PASS (full' in result
  print('PASS matrix N=%d G=%d P=%d full verification'%(n,g,p))
 time.sleep(.2);assert 'Active jobs: 0' in cmd('tools/tshctl','ps')
 print('PASS all matrix workers completed')
 # Inspect interactive tester dispatch via a menu smoke test (PS then Quit).
 result=cmd('tsh/tshtest',str(port),input='7\n\n6\n')
 print('PASS interactive tester PS/quit menu smoke')
finally:
 if daemon.poll() is None:
  try:cmd('tools/tshctl','killall')
  except Exception:pass
  daemon.terminate();daemon.wait(timeout=5)
 for p in children:
  if p.poll() is None:p.kill();p.wait()
 log.close()
