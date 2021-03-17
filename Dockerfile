FROM msrd0/cmake-qt5

WORKDIR /home/user/PrivacyService
ADD . ./

RUN sudo apk add qt5-qtwebsockets-dev

EXPOSE 4242

USER user
ENV HOME /home/user

CMD ["/bin/bash"]