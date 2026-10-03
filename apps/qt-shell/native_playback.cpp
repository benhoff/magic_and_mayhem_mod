#include "native_playback.hpp"
#include <QAudioBuffer>
#include <QAudioDevice>
#include <QVideoFrame>
#include <QUrl>
#include <algorithm>

namespace {
QAudioFormat captureFormat(){QAudioFormat f;f.setSampleRate(22050);f.setChannelCount(2);f.setSampleFormat(QAudioFormat::Int16);return f;}
}
NativePlayback::NativePlayback(bool movie,bool audible):movie_(movie),audible_(audible),audio_(captureFormat()){
    player_.setVideoSink(&video_);player_.setAudioBufferOutput(&audio_);
    if(audible_)player_.setAudioOutput(&output_);
    connect(&video_,&QVideoSink::videoFrameChanged,this,[this](const QVideoFrame& video){
        if(finished_ || !video.isValid())return;
        QVideoFrame mapped(video);pixelFormat_=int(video.pixelFormat());
        QImage image;
        // Qt's RGBX conversion can expose the unused X byte as alpha. Copy the
        // three color bytes explicitly; movie frames are opaque.
        const bool rgbx=video.pixelFormat()==QVideoFrameFormat::Format_RGBX8888;
        const bool bgrx=video.pixelFormat()==QVideoFrameFormat::Format_BGRX8888;
        if((rgbx || bgrx) && mapped.map(QVideoFrame::ReadOnly)){
            image=QImage(video.size(),QImage::Format_RGBA8888);
            if(mapped.bytesPerLine(0)<image.width()*4){mapped.unmap();return;}
            for(int y=0;y<image.height();++y){
                const auto* src=mapped.bits(0)+y*mapped.bytesPerLine(0);auto* dst=image.scanLine(y);
                for(int x=0;x<image.width();++x){dst[4*x]=src[4*x+(rgbx?0:2)];dst[4*x+1]=src[4*x+1];dst[4*x+2]=src[4*x+(rgbx?2:0)];dst[4*x+3]=255;}
            }
            mapped.unmap();
        }else image=video.toImage().convertToFormat(QImage::Format_RGBA8888);
        if(image.isNull())return;
        ++videoFrames_;videoSize_=image.size();
        if(firstFrameHash_.isEmpty()){
            QCryptographicHash hash(QCryptographicHash::Sha256);
            for(int y=0;y<image.height();++y)hash.addData(QByteArrayView(reinterpret_cast<const char*>(image.constScanLine(y)),image.width()*4));
            firstFrameHash_=hash.result().toHex();
            for(int y:{image.height()/4,3*image.height()/4})for(int x:{image.width()/4,3*image.width()/4}){
                const auto color=image.pixelColor(x,y);firstColors_.append(QJsonArray{color.red(),color.green(),color.blue(),color.alpha()});
            }
        }
        if(frame)frame(std::move(image));
    });
    connect(&audio_,&QAudioBufferOutput::audioBufferReceived,this,[this](const QAudioBuffer& buffer){
        if(finished_ || !buffer.isValid())return;
        audioFrames_+=buffer.frameCount();audioBytes_+=buffer.byteCount();
        audioHash_.addData(QByteArrayView(buffer.constData<char>(),buffer.byteCount()));
    });
    connect(&player_,&QMediaPlayer::mediaStatusChanged,this,[this](QMediaPlayer::MediaStatus status){
        if(finished_)return;
        if(status==QMediaPlayer::LoadedMedia || status==QMediaPlayer::BufferedMedia){
            if((movie_ && !player_.hasVideo()) || (!movie_ && !player_.hasAudio())){error_="Required media track is missing.";finish(5);return;}
            if(!accepted_){accepted_=true;if(accepted)accepted();}
        }
        if(status==QMediaPlayer::EndOfMedia)finish(3);
    });
    connect(&player_,&QMediaPlayer::errorOccurred,this,[this](QMediaPlayer::Error,const QString& error){
        if(finished_)return;
        error_=error;finish(5);
    });
}
NativePlayback::~NativePlayback(){finished_=true;player_.stop();player_.setVideoSink(nullptr);player_.setAudioBufferOutput(nullptr);player_.setAudioOutput(nullptr);}
void NativePlayback::start(const QString& path,bool loop){
    path_=path;finished_=false;accepted_=false;result_=0;error_.clear();videoFrames_=0;audioFrames_=audioBytes_=0;
    videoSize_={};pixelFormat_=0;firstColors_={};audioHash_.reset();firstFrameHash_.clear();
    player_.setLoops(loop?QMediaPlayer::Infinite:1);player_.setSource(QUrl::fromLocalFile(path));player_.play();
}
void NativePlayback::finish(unsigned result){
    if(finished_)return;
    finished_=true;result_=result;player_.stop();if(finished)finished(result);
}
void NativePlayback::stop(unsigned result){finish(result);}
void NativePlayback::pause(bool value){if(finished_)return;if(value)player_.pause();else player_.play();}
void NativePlayback::setVolume(float volume){output_.setVolume(std::clamp(volume,0.0f,1.0f));}
QJsonObject NativePlayback::report() const{
    return {{"path",path_},{"result",int(result_)},{"error",error_},{"accepted",accepted_},{"video_frames",int(videoFrames_)},
        {"pixel_format",pixelFormat_},{"first_frame_quadrants",firstColors_},{"width",videoSize_.width()},{"height",videoSize_.height()},{"first_frame_rgba_sha256",QString::fromLatin1(firstFrameHash_)},
        {"audio_frames",audioFrames_},{"audio_bytes",audioBytes_},{"audio_s16le_stereo_22050_sha256",QString::fromLatin1(audioHash_.result().toHex())},
        {"audible_output_requested",audible_},{"audio_device_available",!output_.device().isNull()}};
}
