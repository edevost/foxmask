library(exifr)


indir    <- '~/Documents/foxmask/reconyx'
folder   <- 'Fox024-2008-Cam2-test'
prfixe   <- 'FOX024-BD02_'
obsfile  <- 'tab_2008_FOX024-test'
predfile <- 'Fox024-2008-Cam2'
legende  <- 'Figure 2'


### Get images metadata

fls   <- list.files(paste0(indir, '/', folder), pattern = '\\.JPG$|\\.jpg$', full.names = TRUE)
meta  <- exifr(fls)

files <- gsub('\\.JPG', '', meta[ , 'FileName'])
dates <- gsub('Dat: ', '', unlist(lapply(strsplit(meta[ , 'Comment'], '\n'), function(x) x[grep('^Dat: ', x)])))
dates <- gsub(' AM', '', dates)



### Format dates

pos <- grep('PM', dates)
if (length(pos) > 0){
    tmp  <- strsplit(dates[pos], ' ')
    hour <- strsplit(unlist(lapply(tmp, function(x) x[2])), ':')
    mmss <- unlist(lapply(hour, function(x) paste(x[2], x[3], collapse = ':', sep = ':')))
    hour <- gsub('24', '00', unlist(lapply(hour, function(x) as.numeric(x[1])+12)))
    hour <- paste(hour, mmss, sep = ':')
    dates[pos] <- paste(unlist(lapply(tmp, function(x) x[1])), hour)
}

dates <- as.POSIXct(strptime(dates, '%Y-%m-%d %H:%M:%S'))



### Sort images by dates

pos <- order(dates)
files <- files[pos]
dates <- dates[pos]



### Correspondance table

cam <- paste0(prfixe, gsub('-', '', substr(dates, 1, 10)), '_', gsub(':', '', substr(dates, 12, 16)))
cam <- paste(cam, 1 : length(cam), sep = '_')
dat <- data.frame(file = files, date = dates, name = cam)



### Add manual observations

obs <- read.delim(paste0(obsfile, '.txt'))
obs <- data.frame(photo = obs[ , 'Photo'], renard = obs[ , 'AM'] + obs[ , 'ANM'] + obs[ , 'J'])

obs$renard <- ifelse(obs$renard > 0, 1, 0)
obs$photo  <- as.character(obs$photo)
obs$photo  <- gsub('\\.JPG', '', obs$photo)
obs <- obs[grep(prfixe, obs[ , 'photo']), ]

dat <- merge(dat, obs, by.x = 'name', by.y = 'photo', all = TRUE)
dat <- dat[which(!is.na(dat$renard)), ]



### Sensitivity analysis

minsize <- seq(5, 1000, by = 5)
good1   <- good0 <- gall <- NULL

for (i in 1 : length(minsize)){

    pred <- read.csv(paste0(indir, '/Results/', predfile, '-', minsize[i],'.csv'), header = FALSE)
    colnames(pred) <- c('photo', 'object', 'minsize')

    pred$photo <- as.character(pred$photo)
    pred$photo <- gsub('\\./MasksResults/|\\.png', '', pred$photo)

    tmp <- dat

    tmp <- merge(tmp, pred, by.x = 'file', by.y = 'photo', all = TRUE)
    tmp[ , 'file'] <- as.character(tmp[ , 'file'])
    tmp[ , 'name'] <- as.character(tmp[ , 'name'])
    tmp <- tmp[which(!is.na(tmp$object)), ]

    good1[i] <- 100 * (length(which(tmp$renard == 1 & tmp$object == 1))) / length(unique(which(tmp$renard == 1)))
    good0[i] <- 100 * (length(which(tmp$renard == 0 & tmp$object == 0))) / length(unique(which(tmp$renard == 0)))
    gall[i]  <- 100 * (length(which(tmp$renard == 1 & tmp$object == 1)) +
                       length(which(tmp$renard == 0 & tmp$object == 0))) / nrow(tmp)
}



### Sensitivity graph

pdf(paste0('~/Desktop/', gsub(' ', '', legende), '.pdf'))

par(family = 'serif', xaxs = 'i', yaxs = 'i', mgp = c(3, .75, 0), mar = c(3.5, 4, 1.5, 1))
plot(0, type = 'n', axes = FALSE, ann = FALSE, xlim = c(0, 1000), ylim = c(0, 100))
par(xpd = TRUE)
lines(minsize, good1, lwd = 2, col = '#01665e')
lines(minsize, good0, lwd = 2, col = '#8c510a')
lines(minsize, gall,  lwd = 2, col = '#5e4fa2')
par(xpd = FALSE)
axis(1, seq(0, 1000, 100), seq(0, 1000, 100))
axis(2, seq(0,  100,  10), paste0(seq(0,  100,  10), '%'), las = 1)
mtext(side = 1, line = 2, 'Taille minimale de détection', font = 1)
mtext(side = 2, line = 2.75, 'Pourcentage de bon classement', font = 1)
legend('bottomright', lwd = 2, col = c('#01665e', '#8c510a', '#5e4fa2'),
        c('Animaux', 'Sans animaux', 'Total'), bty = 'n', title = legende)
par(family = 'mono')
text(x = 100, y = 6.0, pos = 4, bquote(paste('N'['(avec animaux)'], ' = ', .(length(unique(which(tmp$renard == 1)))))))
text(x = 100, y = 2.5, pos = 4, bquote(paste('N'['(sans animaux)'], ' = ', .(length(unique(which(tmp$renard == 0)))))))

dev.off()
