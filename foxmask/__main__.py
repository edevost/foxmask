import click
import foxmask
import os
import shutil


@click.command()
@click.option('--foldersdir', help='Directory containing images directories',
              default='images')
@click.option('--resultsdir', help='Directory where to write the final results',
              default='.')
def main(foldersdir, resultsdir):
    """Exectution of the foxmask module.
    """
    if os.path.exists(resultsdir + '/FoxMaskResults'):
        click.confirm(resultsdir + '/FoxMaskResults directory exist Do you want to '
                      'continue and overwrite the results', abort=True)
    foxmask.makeresultsfolder(resultsdir)
    folderslist = foxmask.getfolders(foldersdir)
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
        imageanalysis.bgfgestimation(sortedimglist, impg, foldersdir)
        imageanalysis.getmaskslist(foldersdir)
        imageanalysis.masks_analysis(sortedimglist)
        shutil.rmtree(foldersdir + '/MasksResults')
        imageanalysis.writeresults(item, resultsdir)


if __name__ == "__main__":
    main()
