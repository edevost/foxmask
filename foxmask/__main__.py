import click
import foxmask
import os
import shutil


@click.command()
@click.argument('srcdir', nargs=1, type=click.Path(
    exists=True, dir_okay=True))
@click.argument('resultsdir', nargs=1, type=click.Path(
    exists=True, dir_okay=True))

def main(srcdir, resultsdir):
    """Exectution of the foxmask module.
    """
    if os.path.exists(resultsdir + '/FoxMaskResults'):
        click.confirm(resultsdir + '/FoxMaskResults directory exist Do you want to '
                      'continue and overwrite the results', abort=True)
    foxmask.makeresultsfolder(resultsdir)
    folderslist = foxmask.getfolders(srcdir)
    classdict = {}
    for folder in folderslist:
        classdict[folder] = foxmask.Getimagesinfos(folder)
    for item in classdict:
        print item
        classdict[item].getimageslist()
        classdict[item].getimagesmeta()
        sortedimglist = classdict[item].sortimages()
        print "hello", os.path.basename(sortedimglist[0])
        impg = classdict[item].getimpg()
        imageanalysis = foxmask.Imagesanalysis(classdict[item])
        imageanalysis.bgfgestimation(sortedimglist, impg, srcdir)
        imageanalysis.getmaskslist(srcdir)
        imageanalysis.masks_analysis(sortedimglist)
        shutil.rmtree(srcdir + '/MasksResults')
        imageanalysis.writeresults(item, resultsdir)


if __name__ == "__main__":
    main()
