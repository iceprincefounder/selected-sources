import os,sys

LCA_PROJ_PATH="/mnt/proj/projects"

if not os.environ.get("LCA_ASSETS_SEARCH_PATH"):
    LCA_USD_SEARCH_PATH=LCA_PROJ_PATH
else:
    LCA_USD_SEARCH_PATH=os.environ["LCA_ASSETS_SEARCH_PATH"]

try:
    SGUSD_ROOT_PATH = os.path.dirname(os.path.realpath(__file__))
except:
    SGUSD_ROOT_PATH = "/home/xukai/Git/git_repo/scenegraphUSD/python/scenegraphUSD"
