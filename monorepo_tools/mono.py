
import re
import os
import yaml

from impulse.args import args


command = args.ArgumentParser(complete=True)


MONO_CONFIG = '.mono.yaml'


def load_ignore():
  if not os.path.exists('.gitignore'):
    raise ValueError('No Gitignore!')
  with open('.gitignore') as f:
    return set(x.strip() for x in f.readlines())


def load_config():
  with open(MONO_CONFIG) as f:
    return yaml.load(f.read(), Loader=yaml.SafeLoader)


def indent(lines):
  return '\n  '.join(lines).strip()


def count(lines):
  return len(lines)


def eval_git_command(cmd, eval=indent):
  lines = os.popen('git ' + cmd).read().strip().split('\n')
  return eval([l for l in lines if l])


@command
def status():
  oldcwd = os.getcwd()
  for key, value in load_config().items():
    os.chdir(key)
    eval_git_command('fetch')
    ahead = eval_git_command('log --branches --not --remotes --oneline', count)
    behind = eval_git_command('log --remotes --not --branches --oneline', count)
    status = eval_git_command('status --porcelain')
    if ahead or behind or status:
      print(key)
      print(f'  commits ahead: {ahead}')
      print(f'  commits behind: {behind}')
      if status:
        print(f'  {status}')
      else:
        print('')
    os.chdir(oldcwd)


@command
def init():
  if os.path.exists(MONO_CONFIG):
    exit(1, 'mono repo already initialized')
  os.system(f'touch {MONO_CONFIG}')
  if not os.path.exists('.git'):
    os.system('git init')
    os.system('touch .gitignore')
    return


@command
def new(repo:str, upstream:str=None):
  if os.path.exists(repo):
    raise ValueError(f'Subproject {repo} already exists!')
  if not upstream:
    os.system(f'mkdir -p {repo}')
    os.system(f'cd {repo} && git init')
    os.system(f'echo {repo} >> .gitignore')
  else:
    os.system(f'mkdir -p {repo}')
    os.system(f'git clone {upstream} {repo}')
  with open(MONO_CONFIG, 'a+') as f:
    f.write(f'\n{repo}:\n  source: {upstream}\n')


@command
def sync():
  ignore_set = load_ignore()
  def process_rule(name, source) -> bool:
    if source.startswith('git@'):
      if not os.path.exists(name):
        os.system(f'git clone {source} {name}')
      else:
        oldcwd = os.getcwd()
        os.chdir(name)
        print(f'updating {name} {source}')
        print('  fetching...')
        os.system('git fetch')
        behind = eval_git_command('log --remotes --not --branches --oneline',
                                  count)
        print(f'  commits behind: {behind}')
        if behind:
          if not eval_git_command('status --porcelain'):
            os.system('git pull --rebase')
          else:
            print('  can\'t pull in a dirty repo')
        os.chdir(oldcwd)
    elif source.startswith('symlink://'):
      if not os.path.exists(name):
        os.system(f'ln -s {source[10:]} {name}')
    elif source.startswith('xxx'):
      print(f'XXX => {name}')
    else:
      raise ValueError('Cant handle source: {source}')
    if name not in ignore_set:
      os.system(f'echo {name} >> .gitignore')
      return True
    return False

  data = load_config()
  this_round = set(data.keys())
  finished_rules = set([None])
  next_round = set()
  unchanged = len(this_round) + 1
  needs_commit = False
  while len(this_round) != unchanged:
    unchanged = len(this_round)
    for key in this_round:
      if data[key].get('requires', None) not in finished_rules:
        next_round.add(key)
      else:
        needs_commit = process_rule(key, data[key]['source']) or needs_commit
        finished_rules.add(key)
    this_round = next_round
  if needs_commit:
    os.system('git add .gitignore && git commit -m "mono sync"')


def main():
  command.eval()