import os,socket,subprocess,time,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
with socket.socket() as sock:
    sock.bind(('127.0.0.1',0));port=sock.getsockname()[1]
env=os.environ.copy();env.update(TSHPORT=str(port),TSHHOST='127.0.0.1',MSH_PATH=str(root/'tsh/myshell'),TERM='dumb')
proc=subprocess.Popen([str(root/'tsh/tsh'),str(port)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,cwd=root/'tsh',env=env)
try:
    time.sleep(0.25)
    def run(cmd,input=None):
        x=subprocess.run(cmd, input=input, text=True, capture_output=True, timeout=8,env=env)
        print('RESULT',x.returncode,cmd[-1],repr(x.stdout[-400:]),repr(x.stderr[-200:]))
        return x
    run([str(root/'tests/tuple_probe'),'put','verify_test_label','hello'])
    x=run([str(root/'tsh/tshtest'),str(port)],'3\n^verify_test_label$\n\n6\n')
    assert x.returncode==0
    assert 'TSH_OP_READ' in x.stdout
    assert 'tuple : hello' in x.stdout
    run([str(root/'tests/tuple_probe'),'get','^verify_test_label$'])
    x=run([str(root/'tsh/tshtest'),str(port)],'1\nverify_newput\n1\n16\nworld\n\n6\n')
    assert x.returncode==0
    assert 'TSH_OP_PUT' in x.stdout
    x=run([str(root/'tests/tuple_probe'),'get','^verify_newput$'])
    assert x.stdout.rstrip('\x00') == 'world\n'
    print('PASS READ label, tuple retention, interactive PUT')
finally:
    proc.terminate();
    try:proc.communicate(timeout=5)
    except subprocess.TimeoutExpired:proc.kill()
